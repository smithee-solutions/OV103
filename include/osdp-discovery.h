/*
  osdp-discovery.h - definitions for OSDP discovery tools
  
  (C)2025 Smithee Solutions LLC
*/

#define EQUALS ==

#ifndef OSDP_COMMAND_POLL
#include <osdp-protocol.h>
#endif


#define DISCOVERY_SETTINGS_FILENAME "discovery-settings.json"


typedef struct __attribute__((packed)) osdp_message
{
  unsigned char msg_start;
  unsigned char lth_hi;
  unsigned char lth_lo;
  unsigned char address;
  unsigned char control;
  unsigned char command;
  unsigned char payload_start;
} OSDP_MESSAGE;

typedef struct discovery_timer
{
  unsigned long timer;
} DISCOVERY_TIMER;

typedef struct osdp_discovery_context
{
  int verbosity;
  int fd; // file descriptor for serial port
  char device [1024];
  char speed_s [1024];
  FILE *log;
  struct termios tio;
  unsigned long timer_serial;
  unsigned char buffer [OSDP_MAX_MESSAGE_SIZE];
  int buf_idx;
  int spill_count;
  int overflows;
} OSDP_DISCOVERY_CONTEXT;


int check_serial_input(OSDP_DISCOVERY_CONTEXT *ctx);
void dump_osdp_message(OSDP_DISCOVERY_CONTEXT *ctx, OSDP_MESSAGE *msg, int lth, char *dir_tag);
unsigned short int fCrcBlk(unsigned char *pData, unsigned short int nLength);
int initialize_serial_port(OSDP_DISCOVERY_CONTEXT *ctx);
int length_valid(OSDP_DISCOVERY_CONTEXT *ctx, OSDP_MESSAGE *msg, int length_in_buffer);
unsigned char osdp_discovery_response(OSDP_MESSAGE *msg);
int process_input_message(OSDP_DISCOVERY_CONTEXT *ctx);
int read_settings(OSDP_DISCOVERY_CONTEXT *ctx);
int setup_osdp_mfg_message(OSDP_DISCOVERY_CONTEXT *ctx, int direction, unsigned char *my_OUI, unsigned char mfg_command, unsigned char *detail, int detail_length);
int start_discovery_timer(OSDP_DISCOVERY_CONTEXT *ctx, DISCOVERY_TIMER *current_time);
int time_expired(OSDP_DISCOVERY_CONTEXT *ctx, DISCOVERY_TIMER *duration, DISCOVERY_TIMER *current_timer);


#define ST_OK (0)
#define ST_DISCOVERY_WHOLE_PACKET ( 1)
#define ST_DISCOVERY_SELECT_ERROR ( 2)
#define ST_DISCOVERY_OVERFLOW     ( 3)
#define ST_DISCOVERY_LENGTH       ( 4)
#define ST_DISCOVERY_BAD_CRC      ( 5)
#define ST_DISCOVERY_UNK_PD_MSG   ( 6)
#define ST_DISCOVERY_LTH_ERR      ( 7)
#define ST_DISCOVERY_HDR_LTH      ( 8)
#define ST_DISCOVERY_SERIAL_OPEN_ERR ( 9)
#define ST_DISCOVERY_SERIAL_SET_ERR  (10)
#define ST_DISCOVERY_CANNOT_DISCOVER (11)

