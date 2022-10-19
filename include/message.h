#ifndef _RCOM_MESSAGE_H_
#define _RCOM_MESSAGE_H_

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "set_st.h"
#include "frame_defines.h"
#include "utils.h"

static int rcv_paket_nr = 0;

int sendAndWaitMessage(int fd, unsigned char *msg, int messageSize);

void alarm_handler();

int sendInformationFrame(int fd, const unsigned char *data, int dataSize, int packet);

int readMessageWithResponse(int fd);

void set_rcv_packet_nr(int rcv_paket);

#endif
