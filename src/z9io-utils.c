/*
  z9io-utils - utility routines for Z9IO simulator

  Confidential - Farpointe Data
*/


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <sys/stat.h>
#include <arpa/inet.h>


#include <jansson.h>


#include <z9io-tester.h>




void allocate_random
  (unsigned char *buffer,
  int byte_count)

{ /* allocate_random */

  memset(buffer, 0x17, byte_count);

} /* allocate_random */



/*
  dump_z9io_message - dumps the message in text and hex.

  only dumps hex at verbosity 4 or higher.
*/
void dump_z9io_message
  (Z9IO_TESTER_CONTEXT *ctx,
  Z9IO_MESSAGE *msg, 
  int lth,
  char *dir_tag)

{ /* dump_z9io_message */

  int i;
  unsigned char *msg_raw;


  msg_raw = (unsigned char *)msg;
  fprintf(stderr, "%s ", dir_tag);
  fprintf(stderr,
"SOM=%02X Lth=%3d. A%d SEQ=%03d DISC %02X CRC %02X%02X\n",
    msg->msg_start, 256*(msg->lth_hi) + msg->lth_lo, msg->address, msg->sequence, msg->discriminator,
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

} /* dump_z9io_message */


/*
  this_message_length is the discriminator+payload

  the message length is the actual payload length including the discriminator.
  this writes the header and the discriminator, not the payload or the CRC
*/

int format_z9io_header
  (Z9IO_TESTER_CONTEXT *ctx,
  unsigned char *mbuf,
  int *mbuf_lth,
  unsigned char discriminator,
  unsigned short int message_length,
  unsigned char *sequence)

{ /* format_z9io_header */

  unsigned char *p;
  int lth;
  unsigned short int msg_lth_actual;
  unsigned short int lth_field;


  p = mbuf;
  lth = *mbuf_lth;

  // Add Z9IO Message Start
  *p = Z9IO_MESSAGE_START;
  p++; lth++;

  /*
    calculate actual length:
      SOM (1)
      Lth (2)
      Addr (1)
      Seq (1)
      supplied length (includes discriminator)
      CRC (2)
  */

  msg_lth_actual = 1 + 2 + 1 + 1 + message_length + 2; 
  //fprintf(stderr, "DEBUG: payload 0x%x format lth 0x%x\n", message_length, msg_lth_actual);
  lth_field = htons(msg_lth_actual);
  memcpy(p, &lth_field, sizeof(lth_field));
  p = p + sizeof(lth_field);
  lth = lth + sizeof(lth_field);

  // Z9IO Address
  *p = 0x00;
  p++; lth++;

  // Z9IO Sequence 

  *p = *sequence;
  (*sequence)++;
  p++; lth++;

  // Z9IO Discriminator
  *p = discriminator;
  p++; lth++;

  /*
    count but do not append CRC
  */
  lth = lth + 2;

  *mbuf_lth = lth;
  return(ST_OK);

} /* format_z9io_header */


int initialize_serial_port
  (Z9IO_TESTER_CONTEXT *ctx)

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
    status= ST_Z9IO_SERIAL_OPEN_ERR;
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
      status = ST_Z9IO_SERIAL_SET_ERR;
  };

  return(status);

} /* initialize_serial_port */


int length_valid
  (Z9IO_TESTER_CONTEXT *ctx,
  Z9IO_MESSAGE *msg,
  int length_in_buffer)
{
  int length_in_header;
  int status;


  status = ST_OK;
  length_in_header = 256*msg->lth_hi + msg->lth_lo;
  if (ctx->verbosity > 9)
    fprintf(stderr, "Lth: hdr %d actual %d\n", length_in_header, length_in_buffer);
    
  if (length_in_header != length_in_buffer)
    status = ST_Z9IO_HDR_LTH;
  if (status EQUALS ST_OK)
  {
    //fprintf(stderr, "DEBUG: min %d max %d\n",
    //  Z9IO_MIN_PACKET_SIZE, Z9IO_MAX_PACKET_SIZE);
    if ((length_in_header < Z9IO_MIN_PACKET_SIZE) || (length_in_header > Z9IO_MAX_PACKET_SIZE))
      status = ST_Z9IO_LTH_ERR;
  };
  return(status);
} /* length_valid */


/*
  process_timers_coarse - handles 1-second resolution timer(s)
*/

int process_timers_coarse
  (Z9IO_TESTER_CONTEXT *ctx,
  int timer_number,
  int action,
  int value)

{ /* process_timers_coarse */

  int status;

  status = ST_OK;
  if ((timer_number < Z9IO_TIMER_COARSE_MIN) || (timer_number > Z9IO_TIMER_COARSE_MAX))
    status= ST_Z9IO_TIMER_INVALID;
  if (status EQUALS ST_OK)
  {
    switch(action)
    {
    default:
      status = ST_Z9IO_TIMER_ACTION;
      break;
    case TIMER_COARSE_START_REPEAT:
      ctx->timers_coarse[timer_number].last = time(NULL) + value;
      break;
    case TIMER_COARSE_CHECK:
      if (time(NULL) >= ctx->timers_coarse[timer_number].last)
      {
        ctx->timers_coarse[timer_number].last = time(NULL) + value;
        status = ST_Z9IO_TIMER_EXPIRED;
      };
    };
  };
  return(status); 

} /* process_timers_coarse */


/*
  read_settings - reads settings values from json file

  settings file name is z9io-settings.json, located in current directory.

  parameters:
    cardread-timeout - time in seconds to wait before SC sends another read.
    device - serial device to use
    verbosity - message detail level.  default is 3, debug is 9, silent is 0
*/
int read_settings
  (Z9IO_TESTER_CONTEXT *ctx)

{ /* read_settings */

  int i;
  json_t *root;
  int status;
  json_error_t status_json;
  json_t *value;


  status = ST_OK;
  root = json_load_file(Z9IO_SETTINGS_FILENAME, 0, &status_json);
  if (root != NULL)
  {
    value = json_object_get (root, "verbosity");
    if (json_is_string (value))
    {
      sscanf(json_string_value(value), "%d", &i);
      ctx->verbosity = i;
    };
    if (ctx->verbosity > 3)
      fprintf(stderr, "reading settings from %s\n", Z9IO_SETTINGS_FILENAME);
    value = json_object_get (root, "cardread-timeout");
    if (json_is_string (value))
    {
      sscanf(json_string_value(value), "%d", &i);
      ctx->cardread_timeout = i;
    };
    value = json_object_get (root, "device");
    if (json_is_string (value))
    {
      strcpy(ctx->device, json_string_value(value));
    };
    value = json_object_get (root, "encryption");
    if (json_is_string (value))
    {
      strcpy(ctx->key_exchange_mode, json_string_value(value));
      if (0 EQUALS strcmp(ctx->key_exchange_mode, "auto"))
        ctx->key_exchange = 1;
    };
    value = json_object_get (root, "script");
    if (json_is_string (value))
    {
      strcpy(ctx->external_script, json_string_value(value));
    };
    value = json_object_get (root, "speed");
    if (json_is_string (value))
    {
      strcpy(ctx->speed_s, json_string_value(value));
    };
  };

  return(status);

} /* read_settings */

 
int send_serial_data
  (Z9IO_TESTER_CONTEXT *ctx,
  unsigned char *send_buffer,
  int send_length)

{ /* send_serial_data */

  write (ctx->fd, send_buffer, send_length);

  return(ST_OK);

} /* send_serial_data */


int sequence_valid
  (Z9IO_TESTER_CONTEXT *ctx,
  Z9IO_MESSAGE *msg,
  unsigned char *sequence_checked)

{ /* sequence_valid */

  unsigned char next_sequence;
  int status;


  status = ST_OK;
  next_sequence = *sequence_checked;
  if ((msg->sequence EQUALS 0) && (next_sequence != 0))
  {
    if (ctx->verbosity > 3)
      fprintf(stderr, "DEBUG: msg seq %d not expected, reset to 0\n",
        msg->sequence);
    status = ST_Z9IO_SEQ_ZERO;
  }
  else
  {
    if (msg->sequence != next_sequence)
    {
      if (ctx->verbosity > 3)
        fprintf(ctx->log, "wire %02X expected %02X\n", msg->sequence, *sequence_checked);
      status = ST_Z9IO_SEQ_ERR;
    };
  };
  if ((status EQUALS ST_OK) || (status EQUALS ST_Z9IO_SEQ_ZERO))
  {
    if (ctx->verbosity > 3)
      fprintf(stderr, "incrementing checked sequence from %d\n", *sequence_checked);
    (*sequence_checked) ++;
  };
  return(status);

} /* sequence_valid */

