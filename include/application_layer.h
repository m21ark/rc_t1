#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "macros.h"
#include "set_st.h"
#include "message.h"
#include "link_layer.h"

void al_close_rx();

void al_close_tx();

int al_open_tx(const char *filename_tx);

void al_open_rx(const char *filename_rx);

int readFromFile(char *message_send, unsigned msg_size);

int writeToFile(char *message_rcv, unsigned msg_size);

int makeCtrlPacket(unsigned char ctrlByte, unsigned char *packet, char *filename, int filesize);

int parseCtrlPacket(unsigned char *packetBuffer, int *fileSize, char *fileName);

int makeDataPacket(unsigned char *packet, int seqNum, unsigned char *data, int dataLen);

int parseDataPacket(unsigned char *packet, unsigned char *data);

int sendFile(char *filename);

int rcvFile(char *filename);

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename);

#endif // _APPLICATION_LAYER_H_
