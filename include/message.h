#ifndef _RCOM_MESSAGE_H_
#define _RCOM_MESSAGE_H_

#include "set_st.h"
#include "frame_defines.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"

int sendAndWaitMessage(int fd, unsigned char * msg, int messageSize);

void alarm_handler();

int sendInformationFrame(int fd, unsigned char * data, int dataSize, int packet);

int readMessageWithResponse(int fd);

static int (*set_state_fun)(unsigned char c);


#endif
