/*
  discovery.h - definitions for OSDP discovery tools
  
  (C)2025 Smithee Solutions LLC
*/

#define EQUALS ==

#define LOG stdout

#ifndef OSDP_COMMAND_POLL
#include <osdp-protocol.h>
#endif


#define DISCOVERY_SETTINGS_FILENAME "discovery-settings.json"


typedef struct discovery_timer
{
  unsigned long timer;
} DISCOVERY_TIMER;

#define DYNAD_BACKOFF_INCREMENT (50*1000000L)

#define DYNAD_STATE_UNDISCOVERED (1)

typedef struct dynamic_address_context
{
  int verbosity;

  int discovery_state;

  int fd; // file descriptor for serial port
  char device [1024];
  char speed_s [1024];
  FILE *log;
  struct termios tio;
  unsigned long timer_serial;
  unsigned char send_buffer [OSDP_MAX_MESSAGE_SIZE];
  int send_buffer_length;
  unsigned char receive_buffer [OSDP_MAX_MESSAGE_SIZE];
  int receive_buffer_length;
//  unsigned char message_buffer [OSDP_MAX_MESSAGE_SIZE];
//  int buf_idx;
  int spill_count;
  int overflows;

  // discovery protocol items
  unsigned int random_backoff_count;  

  // context for discovery client
  unsigned char my_pd_address;

  // message parsing
  unsigned char message_address;
  unsigned char message_command;
} DYNAD_CONTEXT;


int check_serial_input(DYNAD_CONTEXT *ctx);
void dump_osdp_message(DYNAD_CONTEXT *ctx, OSDP_MESSAGE *msg, int lth, char *dir_tag);
unsigned short int fCrcBlk(unsigned char *pData, unsigned short int nLength);
int initialize_serial_port(DYNAD_CONTEXT *ctx);
int length_valid(DYNAD_CONTEXT *ctx, OSDP_MESSAGE *msg, int length_in_buffer);
unsigned char osdp_discovery_response(OSDP_MESSAGE *msg);
int process_input_message(DYNAD_CONTEXT *ctx);
int read_settings(DYNAD_CONTEXT *ctx);
int send_serial_data(DYNAD_CONTEXT *ctx, unsigned char *send_buffer, int send_length);
int setup_osdp_mfg_message(DYNAD_CONTEXT *ctx, int direction, unsigned char *my_OUI, unsigned char mfg_command, unsigned char *detail, int detail_length);
int start_discovery_timer(DYNAD_CONTEXT *ctx, DISCOVERY_TIMER *current_time);
int time_expired(DYNAD_CONTEXT *ctx, DISCOVERY_TIMER *duration, DISCOVERY_TIMER *current_timer);


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

