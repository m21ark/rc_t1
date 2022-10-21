#ifndef _APP_LAYER_H_
#define _APP_LAYER_H_

#include "macros.h"
#include "set_st.h"
#include "message.h"
#include "link_layer.h"

void al_close_rx();

void al_close_tx();

int al_open_tx(const char *filename_tx);

void al_open_rx(const char *filename_rx);

int readFromFile(unsigned char *message_send, unsigned msg_size);

int writeToFile(unsigned char *message_rcv, unsigned msg_size);

int makeCtrlPacket(unsigned char ctrlByte, unsigned char *packet, const char *filename, int filesize);

int parseCtrlPacket(unsigned char *packetBuffer, int *fileSize, char *fileName);

int makeDataPacket(unsigned char *packet, int seqNum, unsigned char *data, int dataLen);

int parseDataPacket(unsigned char *packet, unsigned char *data);

int sendFile(const char *filename);

int rcvFile(const char *filename);

void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename);

#endif // _APP_LAYER_H_