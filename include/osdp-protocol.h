/*
  (C)2025 Smithee Solutions LLC
*/

#define OSDP_MIN_MESSAGE_SIZE (1+1+2+1+1) // SOM, Addr, Lth(2), CTL, CMD
#define OSDP_MAX_MESSAGE_SIZE (1440)

#define OSDP_COMMAND         (0x00)
#define OSDP_RESPONSE        (0x80)
#define OSDP_CRC             (0x04)
#define OSDP_RESPONSE_MFGREP (0x90)
#define OSDP_COMMAND_MFG     (0x80)
#define OSDP_MESSAGE_START  (0x53) // SOM
#define OSDP_CONFIG_ADDRESS (0x7F)

#define OSDP_COMMAND_POLL (0x60)

typedef struct __attribute__((packed)) osdp_message
{
  unsigned char msg_start;
  unsigned char address;
  unsigned char lth_lo;
  unsigned char lth_hi;
  unsigned char control;
  unsigned char command;
  unsigned char payload_start;
} OSDP_MESSAGE;

