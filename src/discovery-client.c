/*
  discovery-client - code runs in a PD before OSDP PD.
*/


#include <stdio.h>
#include <termios.h>
#include <time.h>
#include <string.h>


#include <discovery-protocol.h>
#include <discovery.h>
unsigned int generate_random(DYNAD_CONTEXT *ctx);
int initialize_discovery_client(DYNAD_CONTEXT *ctx);
int process_server_command(DYNAD_CONTEXT *ctx);
int respond_to_discover(DYNAD_CONTEXT *ctx);


int main
  (int argc,
  char *argv [])

{ /* main for discovery-client */

  struct timespec backoff_time;
  DYNAD_CONTEXT *ctx;
  int done;
  DYNAD_CONTEXT dynad_client_context;
  int ignore;
  int status;


  status = ST_OK;
  ctx = &dynad_client_context;
  status = initialize_discovery_client(ctx);
  done = 0;

  if (status EQUALS ST_OK)
  {
    ignore = 0;
    while (!done)
    {
fprintf(stderr, "send index before check serial: %d\n", ctx->send_buffer_length);
      status = check_serial_input(ctx);
 
      if (status EQUALS ST_DISCOVERY_WHOLE_PACKET)
      {
        status = process_server_command(ctx);
        if (ctx->message_address != OSDP_CONFIG_ADDRESS)
        {
          if (ctx->message_address != ctx->my_pd_address)
            ignore = 1;
          else
          {
            if (ctx->verbosity > 2)
              fprintf(LOG, "Discovery client: directed command detected, stopping.\n");
            done = 1;
          };
        }
        else
        {
          // it was for the config address.

          if (ctx->message_command EQUALS DYNAD_START_DISCOVER)
          {
            if (ctx->verbosity > 2)
              fprintf(LOG, "OSDP Discovery Client detected START DISCOVERY\n");
          };
          if (ctx->message_command EQUALS DYNAD_DISCOVER)
          {
            ctx->random_backoff_count = 0x0F & generate_random(ctx);
            if (ctx->verbosity > 2)
              fprintf(LOG, "Discovery backoff %d\n", ctx->random_backoff_count);

            backoff_time.tv_sec = 0;
            backoff_time.tv_nsec = DYNAD_BACKOFF_INCREMENT * ctx->random_backoff_count;
            nanosleep(&backoff_time, NULL);
            status = respond_to_discover(ctx);
          };
          if (ctx->message_command EQUALS DYNAD_DISCOVER_SET)
          {
            if (ctx->verbosity > 2)
              fprintf(LOG, "Discovery: address set to %02X\n", ctx->my_pd_address);
fprintf(LOG, "output config here.\n");
            done = 1;
            status = ST_OK;
          };
        };
      };

      if (ignore)
        status = ST_OK;

      if (status != ST_OK)
        done = 1;
    };
  };
  return(status);

} /* main for discovery-client */


unsigned int generate_random
  (DYNAD_CONTEXT *ctx)
{
  return(1);
}


int initialize_discovery_client
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

//todo client read timeout?
ctx->timer_serial = 50000000;
//ctx->timer_nsec = 999999999;

  fprintf(LOG, "OSDP Discovery Client is in start-up.\n");
//zzz read last pd address from settings?
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
  if (status EQUALS ST_OK)
    ctx->discovery_state = DYNAD_STATE_UNDISCOVERED;
  return (status);

} /* initialize */


/*
  process_server_command - parse a message from the discovery server
*/

int process_server_command
  (DYNAD_CONTEXT *ctx)

{ /* process_server_command */

  OSDP_MESSAGE *osdp_command;

  osdp_command = (OSDP_MESSAGE *)(ctx->receive_buffer);
  ctx->message_address = osdp_command->address & 0x7F;
  ctx->message_command = *(&(osdp_command->payload_start) - 1 + 3);
  return(ST_OK);

} /* process_server_command */


int respond_to_discover
  (DYNAD_CONTEXT *ctx)
{
  return(-1);
}

