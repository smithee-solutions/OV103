/*
  discovery-osdp - osdp routines for discovery

  (C)2025 Smithee Solutions LLC
*/


#include <stdio.h>
#include <termios.h>
#include <string.h>


#include <discovery.h>
#include <discovery-protocol.h>


/*
  direction is OSDP_COMMAND if we're sending a command and OSDP_RESPONSE if we're sending a response.
*/

int setup_osdp_mfg_message
  (DYNAD_CONTEXT *ctx,
  int direction,
  unsigned char *my_OUI,
  unsigned char mfg_command,
  unsigned char *detail,
  int detail_length)

{ /* setup_osdp_mfg_message */

  int idx;
  int message_length;
  int status;


  status = ST_OK;
  memset(ctx->send_buffer, 0, sizeof(ctx->send_buffer));
  idx = 0;
  ctx->send_buffer [idx] = OSDP_MESSAGE_START;
  idx++;
  ctx->send_buffer [idx] = OSDP_CONFIG_ADDRESS;
  if (direction EQUALS OSDP_RESPONSE)
  ctx->send_buffer [idx] = ctx->send_buffer [idx] | OSDP_RESPONSE;
  idx++;
  message_length =
    1 + // SOM
    2 + // address
    1 + // control
    1 + //command
    3 + // OUI
    1 + // MFG command
    2 + // detail length
    2 + // CRC
    detail_length;
  ctx->send_buffer [idx] = 0xff & message_length;
  ctx->send_buffer [idx+1] = (message_length >> 8);
  idx = idx + 2;
  ctx->send_buffer [idx] = OSDP_CRC; // sequence 0, crc packet
  idx++;
  if (direction EQUALS OSDP_COMMAND)
    ctx->send_buffer [idx] = OSDP_COMMAND_MFG;
  else
    ctx->send_buffer [idx] = OSDP_RESPONSE_MFGREP;
  idx++;
  ctx->send_buffer [idx] = mfg_command;
  idx++;
  memcpy(ctx->send_buffer+idx, my_OUI, 3);
  idx = idx + 3;
  ctx->send_buffer [idx] = 0xff & detail_length;
  ctx->send_buffer [idx+1] = (detail_length >> 8);
  idx = idx + 2;
  if (detail_length > 0)
  {
    memcpy(ctx->send_buffer+idx, detail, detail_length);
    idx = idx + detail_length;
  };
  *(unsigned short int *)(ctx->send_buffer + idx) = fCrcBlk(ctx->send_buffer, message_length);
  idx = idx + 2;

  ctx->send_buffer_length = message_length;
  return(status);
}

