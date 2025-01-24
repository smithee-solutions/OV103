/*
  (C)2025 Smithee Solutions LLC
*/

#define OSDP_MIN_MESSAGE_SIZE (1+1+2+1+1) // SOM, Addr, Lth(2), CTL, CMD
#define OSDP_MAX_MESSAGE_SIZE (1440)

#define OSDP_COMMAND  (0x00)
#define OSDP_RESPONSE (0x80)

#define OSDP_MESSAGE_START  (0x53) // SOM
#define OSDP_CONFIG_ADDRESS (0x7F)

#define OSDP_COMMAND_POLL (0x60)


