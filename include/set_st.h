#ifndef _SET_ST_H_
#define _SET_ST_H_

#define EXIT_SET_STATE stop
#define ENTRY_SET_STATE start

#define BUF_SIZE 256

#include "frame_defines.h"
#include "utils.h"
#include <string.h>

// Based on https://stackoverflow.com/questions/1371460/state-machines-tutorials

int set_entry_state(unsigned char c);

int set_flag_state(unsigned char c);

int set_a_state(unsigned char c);

int set_c_state(unsigned char c);

int set_bcc_state(unsigned char c);

int set_stop_state(unsigned char c);

int set_data_state(unsigned char c);

enum set_state_codes
{
  start,
  flag_rcv,
  a_rcv,
  c_rcv,
  bcc_ok,
  stop,
  data
};

enum set_ret_codes
{
  FLAG_RCV,
  A_RCV,
  C_RCV,
  BCC_OK,
  OTHER_RCV,
  INF_FRAME,
  BCC2_NOT_OK
};

typedef struct set_transition
{
  enum set_state_codes src_state;
  enum set_ret_codes ret_code;
  enum set_state_codes dst_state;
} SET_ST_TRANS;

enum set_state_codes set_lookup_transitions(int cur_state, int rc);

enum set_state_codes get_set_state();

void set_set_state(enum set_state_codes st);

unsigned char get_control();
int get_data_size();
void get_data(unsigned char dt[]);

#endif
