/*
  discovery-server - OSDP discovery "server" (ACU)
*/


#include <stdio.h>
#include <string.h>
#include <termios.h>


#include <osdp-discovery.h>
#include <osdp-discovery-version.h>


int initialize(OSDP_DISCOVERY_CONTEXT *ctx);


int main
  (int argc,
  char *argv [])

{ /* main for discovery-server */

  OSDP_DISCOVERY_CONTEXT discovery_context;
  OSDP_DISCOVERY_CONTEXT *ctx;
  int done;
  int status;


  ctx = &discovery_context;
  memset(ctx, 0, sizeof(&ctx));
  status = ST_OK;
  fprintf(stdout, "OSDP Discovery Server (ACU) %s\n", OSDP_DISCOVERY_VERSION);
  done = 0;
  status = initialize(ctx);
  if (status != ST_OK)
    done = 1;
  if (status EQUALS ST_OK)
  {
    while (!done)
    {
      if (status EQUALS ST_OK)
        status = check_serial_input(ctx);
      if (status EQUALS ST_DISCOVERY_WHOLE_PACKET)
      {
        status = process_input_message(ctx);
      };
      if (status != ST_OK)
        done = 1;
    };
  };

  if (status != ST_OK)
  {
    fprintf(stdout, "discovery-server exit status %d.\n", status);
  };
  return(status);

} /* main for discovery-server */


int initialize
  (OSDP_DISCOVERY_CONTEXT *ctx)
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

  fprintf(stdout, "OSDP Discovery Server is in start-up.\n");
  status = read_settings(ctx);

  if (status EQUALS ST_OK)
  {
    fprintf(stdout, "         Serial port: %s\n", ctx->device);
//    fprintf(stdout, "Read wait timer (ns): %ld\n", ctx->timer_nsec);
    fprintf(stdout, "                 Log: %s\n", "stderr");
    fprintf(stdout, "           Verbosity: %d\n", ctx->verbosity);
  };

  if (status EQUALS ST_OK)
  {
    status = initialize_serial_port(ctx);
  };
  return (status);

} /* initialize */

