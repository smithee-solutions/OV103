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
  unsigned char *mfg_payload;
  int status;
  OSDP_MESSAGE *osdp_msg;


  status = ST_OK;
  memset(ctx->send_buffer, 0, sizeof(ctx->send_buffer));

  osdp_msg = (OSDP_MESSAGE *)(ctx->send_buffer);
  osdp_msg->msg_start = OSDP_MESSAGE_START;
  osdp_msg->address = OSDP_CONFIG_ADDRESS;
  if (direction EQUALS OSDP_RESPONSE)
    osdp_msg->address = osdp_msg->address | OSDP_RESPONSE;
  message_length = sizeof(OSDP_MESSAGE) - 1 + 2 + 3 + 2 + 1 + detail_length;
  osdp_msg->lth_lo = 0xff & message_length;
  osdp_msg->lth_hi = (message_length >> 8);
  osdp_msg->control = OSDP_CRC; // sequence 0, crc packet
  if (direction EQUALS OSDP_COMMAND)
    osdp_msg->command = OSDP_COMMAND_MFG;
  else
    osdp_msg->command = OSDP_RESPONSE_MFGREP;

  idx = 0;
  mfg_payload = &(osdp_msg->payload_start);
  memcpy(mfg_payload, my_OUI, 3);
  idx = idx + 3;
  *(mfg_payload+idx) = mfg_command;
  idx++;
  *(mfg_payload+idx) = 0xff & detail_length;
  idx++;
  *(mfg_payload+idx) = (detail_length > 8);
  idx++;
  if (detail_length > 0)
  {
    memcpy(mfg_payload+idx, detail, detail_length);
    idx = idx + detail_length;
  };
  *(unsigned short int *)(mfg_payload + idx) = fCrcBlk(ctx->send_buffer, message_length);

  ctx->send_buffer_length = message_length;
  return(status);

} /* setup_osdp_mfg_message */

