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
  unsigned char osdp_message_buffer [OSDP_MAX_MESSAGE_SIZE];
  int status;


  status = ST_OK;
  memset(osdp_message_buffer, 0, sizeof(osdp_message_buffer));
  idx = 0;
  osdp_message_buffer [idx] = OSDP_MESSAGE_START;
  idx++;
  osdp_message_buffer [idx] = OSDP_CONFIG_ADDRESS;
  if (direction EQUALS OSDP_RESPONSE)
  osdp_message_buffer [idx] = osdp_message_buffer [idx] | OSDP_RESPONSE;
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
  osdp_message_buffer [idx] = 0xff & message_length;
  osdp_message_buffer [idx+1] = (message_length >> 8);
  idx = idx + 2;
  osdp_message_buffer [idx] = OSDP_CRC; // sequence 0, crc packet
  idx++;
  if (direction EQUALS OSDP_COMMAND)
    osdp_message_buffer [idx] = OSDP_COMMAND_MFG;
  else
    osdp_message_buffer [idx] = OSDP_RESPONSE_MFGREP;
  idx++;
  osdp_message_buffer [idx] = mfg_command;
  idx++;
  memcpy(osdp_message_buffer+idx, my_OUI, 3);
  idx = idx + 3;
  osdp_message_buffer [idx] = 0xff & detail_length;
  osdp_message_buffer [idx+1] = (detail_length >> 8);
  idx = idx + 2;
  if (detail_length > 0)
  {
    memcpy(osdp_message_buffer+idx, detail, detail_length);
    idx = idx + detail_length;
  };
  *(unsigned short int *)(osdp_message_buffer + idx) = fCrcBlk(osdp_message_buffer, message_length);
  idx = idx + 2;

  memcpy(ctx->message_buffer, osdp_message_buffer, message_length);
  return(status);
}

