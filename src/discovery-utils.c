/*
  (C)2025 Smithee Solutions LLC
*/

#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>


#include <osdp-discovery.h>
#if 0
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#endif


#include <jansson.h>


int add_octet_and_validate_buffer
  (OSDP_DISCOVERY_CONTEXT *ctx,
  unsigned char wire_octet)

{ /* add_octet_and_validate_buffer */

  int header_reported_length;
  OSDP_MESSAGE *msg;
  int offset;
  int status;


  status = ST_OK;
  msg = (OSDP_MESSAGE *)ctx->buffer;

  if (ctx->buf_idx < OSDP_MAX_MESSAGE_SIZE)
  {
    if (ctx->verbosity > 9)
      fprintf(stderr, "Adding %02X to index %03d.\n", wire_octet, ctx->buf_idx);
    ctx->buffer [ctx->buf_idx] = wire_octet;
    ctx->buf_idx ++;
  }
  else
  {
    fprintf(stderr, "input buffer overflow, dumping input.\n");
    ctx->buf_idx = 0;
    ctx->overflows++;
    status = ST_DISCOVERY_OVERFLOW;
  };
  if (ctx->buffer [0] != OSDP_MESSAGE_START)
  {
    if (ctx->verbosity > 3)
      fprintf(stderr, "DEBUG: dumping first octet (%02X)\n", ctx->buffer[0]);
    ctx->spill_count++;
    memcpy(ctx->buffer, ctx->buffer+1, ctx->buf_idx-1);
    ctx->buf_idx = ctx->buf_idx - 1;
  }
  else
  {
    if (ctx->buf_idx > 2)
    {
      header_reported_length = msg->lth_hi*256 + msg->lth_lo;
      if ((msg->lth_hi*256 + msg->lth_lo) > OSDP_MAX_MESSAGE_SIZE)
      {
        offset = 3; // skip alleged count
        memcpy(ctx->buffer, ctx->buffer+offset, ctx->buf_idx-offset);
        ctx->buf_idx = ctx->buf_idx - offset;
      };
      if (ctx->buf_idx > 3)
      {
        if (msg->address != 0)
        {
          offset = 4; // skip alleged address
          memcpy(ctx->buffer, ctx->buffer+offset, ctx->buf_idx-offset);
          ctx->buf_idx = ctx->buf_idx - offset;
        }
        else
        {
          if (header_reported_length EQUALS ctx->buf_idx)
            status = ST_DISCOVERY_WHOLE_PACKET;
        };
      };
    };
  };

  return(status);

} /* add_octet_and_validate_buffer */


/*
  check_serial_input - check for and accept serial input

  updates ctx->buffer, buf_idx if there was data.
*/
int check_serial_input
  (OSDP_DISCOVERY_CONTEXT *ctx)

{ /* check_serial_input */

  fd_set exceptfds;
  fd_set readfds;
  int scount;
  const sigset_t sigmask;
  fd_set writefds;
  int status;
  int status_io;
  int status_select;
  struct timespec timeout;


  status = ST_OK;

  // do a select waiting for RS-485 serial input (or a HUP)

  FD_ZERO (&readfds);
  FD_SET (ctx->fd, &readfds);
  scount = 1 + ctx->fd;
  FD_ZERO (&writefds);
  FD_ZERO (&exceptfds);
  timeout.tv_sec = 0;
  timeout.tv_nsec = ctx->timer_serial;
  status_select = pselect (scount, &readfds, &writefds, &exceptfds, &timeout, &sigmask);

  // if there was data at the 485 file descriptor, process it.

  if (status_select > 0)
  {
    if (FD_ISSET (ctx->fd, &readfds))
    {
      unsigned char wire_octet [2];

      status_io = read (ctx->fd, wire_octet, 1);
      if (status_io < 1)
      {
        // continue if it was a serial error
        status = ST_OK;
      }
      else
      {
        status = add_octet_and_validate_buffer(ctx, wire_octet [0]);
      };
    };
  }
  else
    if (status_select EQUALS -1)
    {
      status = ST_DISCOVERY_SELECT_ERROR;
    };
  return(status);

} /* check_serial_input */


/*
  dump_osdp_message - dumps the message in text and hex.

  only dumps hex at verbosity 4 or higher.
*/
void dump_osdp_message
  (OSDP_DISCOVERY_CONTEXT *ctx,
  OSDP_MESSAGE *msg, 
  int lth,
  char *dir_tag)

{ /* dump_osdp_message */

  int i;
  unsigned char *msg_raw;


  msg_raw = (unsigned char *)msg;
  fprintf(stderr, "%s ", dir_tag);
  fprintf(stderr,
"SOM=%02X Lth=%3d. A%d CTL=%02X CMD %02X CRC %02X%02X\n",
    msg->msg_start, 256*(msg->lth_hi) + msg->lth_lo, msg->address, msg->control, msg->command,
    *(msg_raw + lth - 2), *(msg_raw + lth - 1));
  if (ctx->verbosity > 3)
  {
    for (i=0; i<lth; i++)
    {
      fprintf(stderr, "%02X", *(msg_raw +i));
      if ((15 EQUALS (i % 16)) && ((lth % 16) != 0))
        fprintf(stderr, "\n");
    };
    fprintf(stderr, "\n");
  };

} /* dump_osdp_message */


int initialize_serial_port
  (OSDP_DISCOVERY_CONTEXT *ctx)

{ /* initialize_serial_port */

  int serial_speed_cfg_value;
  int status;
  int status_io;

  status = ST_OK;

  ctx->fd = open (ctx->device, O_RDWR | O_NONBLOCK);
  if (ctx->verbosity > 3)
    fprintf (stdout, "Opening %s, fd=%d.\n", ctx->device, ctx->fd);
  if (ctx->fd EQUALS -1)
  {
    if (ctx->verbosity > 3)
      fprintf (stderr, "errno at device %s open %d\n", ctx->device, errno);
    status= ST_DISCOVERY_SERIAL_OPEN_ERR;
  }
  if (status EQUALS ST_OK)
  {
    status_io = tcgetattr (ctx->fd, &(ctx->tio));
    cfmakeraw (&(ctx->tio));
    status_io = tcsetattr (ctx->fd, TCSANOW, &(ctx->tio));
  };
  if (status EQUALS ST_OK)
  {
    serial_speed_cfg_value = B9600;
    if (0 EQUALS strcmp(ctx->speed_s, "115200"))
      serial_speed_cfg_value = B115200;
    fprintf(stderr, "Speed is %s\n", ctx->speed_s);
    status_io = cfsetispeed (&(ctx->tio), serial_speed_cfg_value);
    if (status_io EQUALS 0)
      status_io = cfsetospeed (&(ctx->tio), serial_speed_cfg_value);
    if (status_io EQUALS 0)
      status_io = tcsetattr (ctx->fd, TCSANOW, &(ctx->tio));
    if (status_io != 0)
      status = ST_DISCOVERY_SERIAL_SET_ERR;
  };

  return(status);

} /* initialize_serial_port */


int length_valid
  (OSDP_DISCOVERY_CONTEXT *ctx,
  OSDP_MESSAGE *msg,
  int length_in_buffer)
{
  int length_in_header;
  int status;


  status = ST_OK;
  length_in_header = 256*msg->lth_hi + msg->lth_lo;
  if (ctx->verbosity > 9)
    fprintf(stderr, "Lth: hdr %d actual %d\n", length_in_header, length_in_buffer);
    
  if (length_in_header != length_in_buffer)
    status = ST_DISCOVERY_HDR_LTH;
  if (status EQUALS ST_OK)
  {
    if ((length_in_header < OSDP_MIN_MESSAGE_SIZE) || (length_in_header > OSDP_MAX_MESSAGE_SIZE))
      status = ST_DISCOVERY_LTH_ERR;
  };
  return(status);

} /* length_valid */


unsigned char osdp_discovery_response
  (OSDP_MESSAGE *msg)
{
  unsigned char returned_response_code;

  returned_response_code = 0x00;
// pluck response code from mfgrep
  return(returned_response_code);
}

/*
  read_settings - reads settings values from json file

  settings file name is discovery-settings.json, located in current directory.

  parameters:
    device - serial device to use
    speed - serial device speed
    verbosity - message detail level.  default is 3, debug is 9, silent is 0
*/
int read_settings
  (OSDP_DISCOVERY_CONTEXT *ctx)

{ /* read_settings */

  int i;
  json_t *root;
  int status;
  json_error_t status_json;
  json_t *value;


  status = ST_OK;
  root = json_load_file(DISCOVERY_SETTINGS_FILENAME, 0, &status_json);
  if (root != NULL)
  {
    value = json_object_get (root, "verbosity");
    if (json_is_string (value))
    {
      sscanf(json_string_value(value), "%d", &i);
      ctx->verbosity = i;
    };
    if (ctx->verbosity > 3)
      fprintf(stderr, "reading settings from %s\n", DISCOVERY_SETTINGS_FILENAME);
    value = json_object_get (root, "device");
    if (json_is_string (value))
    {
      strcpy(ctx->device, json_string_value(value));
    };
    value = json_object_get (root, "speed");
    if (json_is_string (value))
    {
      strcpy(ctx->speed_s, json_string_value(value));
    };
  };

  return(status);

} /* read_settings */


int start_discovery_timer
  (OSDP_DISCOVERY_CONTEXT *ctx,
  DISCOVERY_TIMER *current_time)

{ /* start_discovery_timer */

  memset((void *)current_time, 0, sizeof(*current_time));
  return(ST_OK);

} /* start_discovery_timer */


// returns 0 for not expired, 1 for expired

int time_expired
  (OSDP_DISCOVERY_CONTEXT *ctx,
  DISCOVERY_TIMER *duration,
  DISCOVERY_TIMER *current_timer)

{
fprintf(stderr, "DEBUG: if now-current time is duration beyond current_timer then return expired (1)\n");
  return(0);
}



