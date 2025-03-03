/*
  discovery-server - OSDP discovery "server" (ACU)
*/


#include <stdio.h>
#include <string.h>
#include <termios.h>


#include <discovery.h>
#include <discovery-protocol.h>
#include <discovery-version.h>


int initialize(DYNAD_CONTEXT *ctx);

unsigned char my_OUI [] = {0x0A, 0x00, 0x17};
DISCOVERY_TIMER TIMER_DISCOVER_WAIT = {1600L}; // milliseconds i.e. 8x poll time


int main
  (int argc,
  char *argv [])

{ /* main for discovery-server */

  DISCOVERY_TIMER current_timer;
  DYNAD_CONTEXT discovery_context;
  DYNAD_CONTEXT *ctx;
  int done;
  int status;


  ctx = &discovery_context;
  memset(ctx, 0, sizeof(&ctx));
  status = ST_OK;
  fprintf(LOG, "OSDP Discovery Server (ACU) %s\n", OSDP_DISCOVERY_VERSION);
  done = 0;
  status = initialize(ctx);
  if (status != ST_OK)
    done = 1;

  // send 'start discover' and confirm nobody answers.
  if (status EQUALS ST_OK)
    status = setup_osdp_mfg_message(ctx, OSDP_COMMAND, my_OUI, DYNAD_START_DISCOVER, NULL, 0);
  if (status EQUALS ST_OK)
    status = send_serial_data(ctx, ctx->send_buffer, ctx->send_buffer_length);
  if (status EQUALS ST_OK)
    status = start_discovery_timer(ctx, &current_timer);
  if (status EQUALS ST_OK)
  {
    while (!done)
    {
      status = check_serial_input(ctx);
 
      // any whole response or an issue means we cannot do discovery.

      if ((status EQUALS ST_DISCOVERY_WHOLE_PACKET) ||
        (ctx->receive_buffer_length > 0) || (ctx->spill_count > 0))
        status = ST_DISCOVERY_CANNOT_DISCOVER;
      if (status EQUALS ST_OK)
      {
        if (time_expired(ctx, &TIMER_DISCOVER_WAIT, &current_timer))
          done = 1;
      };
      if (status != ST_OK)
        done = 1;
    };
  };
  if (status EQUALS ST_DISCOVERY_CANNOT_DISCOVER)
  {
    fprintf(LOG, "Problem issuing device discovery.\n");
  };
  if (status EQUALS ST_OK)
  {
fprintf(stderr, "DEBUG: zzz send the discover command\n");
    status = setup_osdp_mfg_message(ctx, OSDP_COMMAND, my_OUI, DYNAD_DISCOVER, NULL, 0);
  };

//zzz if response do something with it
//zzz do set command
//zzz check for ack

//        status = process_input_message(ctx);

  if (status != ST_OK)
  {
    fprintf(LOG, "discovery-server exit status %d.\n", status);
  };
  return(status);

} /* main for discovery-server */


int initialize
  (DYNAD_CONTEXT *ctx)
{
  int status;


  status = ST_OK;
  memset(ctx, 0, sizeof(*ctx));

  // initialize defaults
  ctx->log = stderr;
  ctx->verbosity = 3;
  strcpy(ctx->device, "/dev/ttyUSB0");
  strcpy(ctx->speed_s, "9600");

  // server read timeout?
  ctx->timer_serial = 50000000;
  //ctx->timer_nsec = 999999999;

  fprintf(LOG, "OSDP Discovery Server is in start-up.\n");
  status = read_settings(ctx);

  if (status EQUALS ST_OK)
  {
    fprintf(LOG, "         Serial port: %s\n", ctx->device);
//    fprintf(LOG, "Read wait timer (ns): %ld\n", ctx->timer_nsec);
    fprintf(LOG, "                 Log: %s\n", "stderr");
    fprintf(LOG, "           Verbosity: %d\n", ctx->verbosity);
  };

  if (status EQUALS ST_OK)
  {
    status = initialize_serial_port(ctx);
  };
  return (status);

} /* initialize */


int process_input_message
  (DYNAD_CONTEXT *ctx)

{ /* process_input_message */

  unsigned short int calc_crc;
  int length;
  OSDP_MESSAGE *osdp_msg;
  unsigned char *p;
  int status;
  unsigned short int wire_crc;
#if 0
  Z9IO_BOARD_INFO *board_info;
  Z9IO_BOARD_STATUS *board_status;
  Z9IO_RAW_READ_MESSAGE *cardread;
  Z9IO_ENCRYPTION_KEY_EXCHANGE *enc_kex;
  int i;
  unsigned char msg_sent [1024];
#endif


  status = ST_OK;
  osdp_msg = (OSDP_MESSAGE *)(ctx->receive_buffer);
  length = ctx->receive_buffer_length;
  if (ctx->verbosity > 3)
  {
    fprintf(LOG, "Discovery Server input received %d octets\n", length);
  };
  dump_osdp_message(ctx, osdp_msg, length, "PD->ACU");

  if (length_valid(ctx, osdp_msg, length) != ST_OK)
    status = ST_DISCOVERY_LENGTH;
  if (status EQUALS ST_OK)
  {
    p = (unsigned char *)osdp_msg;
    p = p + length - 2;
    calc_crc = fCrcBlk((unsigned char *)osdp_msg, length-2);
    wire_crc = *(unsigned short int *)p;
    if (calc_crc != wire_crc)
      status = ST_DISCOVERY_BAD_CRC;
  };
  if (status EQUALS ST_OK)
  {
    switch(osdp_discovery_response(osdp_msg))
    {
    default:
      status = ST_DISCOVERY_UNK_PD_MSG;
      if (ctx->verbosity > 3)
        fprintf(LOG, "Unknown response %02X received.\n", osdp_msg->command);
      break;
    };
  };
  if (status EQUALS ST_OK)
  {
    ctx->receive_buffer_length = 0;
  };
  return(status);

} /* process_input_message */

