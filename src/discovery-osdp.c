/*
  discovery-osdp - osdp routines for discovery

  (C)2025 Smithee Solutions LLC
*/


#include <stdio.h>
#include <termios.h>
#include <string.h>


#include <osdp-discovery.h>
#include <discovery-protocol.h>


int setup_osdp_mfg_message
  (OSDP_DISCOVERY_CONTEXT *ctx,
  int direction,
  unsigned char *my_OUI,
  unsigned char mfg_command,
  unsigned char *detail,
  int detail_length)

{ /* setup_osdp_mfg_message */

  unsigned char osdp_message_buffer [OSDP_MAX_MESSAGE_SIZE];
  int status;


  status = ST_OK;
  memset(osdp_message_buffer, 0, sizeof(osdp_message_buffer));
fprintf(stderr, "DEBUG: finish coding...\n"); status = -1;
#if 0
  set index to 0
  add SOM at index
  increment index
  add CONFIG_ADDRESS as address
  if direction is PD add 0x80
  increment index
  calc length = som + command + 2length + ctl + command=mfg + 3OUI + mfg-command + detail length + 2
  add length
  increment index by 2
  add CTL, sequence 0, no SCS
  increment index
  if direction is ACU add MFG else add MFGREP
  increment index
  add mfg command
  increment index
  add OUI
  increment by 3
  calc internal length from detail length
  add internal length
  increment by 2
  if details length > 0
    add details
    increment index by details length
  calc crc 
  increment index by 2
  copy message buffer to context message buffer
#endif
  return(status);
}

