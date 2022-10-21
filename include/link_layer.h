#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>

#include "message.h"
#include "macros.h"
#include "set_st.h"

/**
 * @brief Possible roles for the serial port are Rx (Reciever) and Tx (Transmitter)
 *
 */
typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

/**
 * @brief Struct that holds serial port settings
 *
 */
typedef struct
{
    char serialPort[50];
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;
} LinkLayer;

struct termios oldtio; // struct to store old serial port settings to allow settings reverting at llclose
int fd;                // global file descriptor used by the serial port

/**
 * @brief Open a connection using the "port" parameters defined in struct linkLayer.
 *
 * @param connectionParameters serial port parameters to be set
 * @return Return "1" on success or "-1" on error.
 */
int llopen(LinkLayer connectionParameters);

/**
 * @brief Send data in buf with size bufSize
 *
 * @param buf buffer containing message to be sent
 * @param bufSize size of message being sent
 * @return Return number of chars written, or "-1" on error.
 */
int llwrite(const unsigned char *buf, int bufSize);

/**
 * @brief Receive data in packet.
 *
 * @param packet message to be read
 * @return Return number of chars read, or "-1" on error.
 */
int llread(unsigned char *packet);

/**
 * @brief Close previously opened connection.
 *
 * @param showStatistics if TRUE, link layer should print statistics in the console on close.
 * @return Return "1" on success or "-1" on error.
 */
int llclose(int showStatistics);

#endif // _LINK_LAYER_H_