// Application layer protocol header.
// NOTE: This file must not be changed.

#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

// 60 64
// 256 252
#define MAXSIZE_DATA 4092 // Max size of AL Data Packet
#define MAXSIZE_FRAME 4096 // Max size of Individual Frame
// FRAME > DATA ? MAXSIZE_FRAME = MAXSIZE_DATA + 4

#define SEQUENCE_MODULO 16 // packet sending rotation to know if out of order

#define CTRL_DATA 0x01
#define CTRL_START 0x02
#define CTRL_END 0x03

#define TYPE_FILESIZE 0x00
#define TYPE_FILENAME 0x01
#define MAXSIZE_FILE_NAME 127

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "frame_defines.h"
#include "set_st.h"
#include "message.h"
#include "link_layer.h"

// Application layer main function.
// Arguments:
//   serialPort: Serial port name (e.g., /dev/ttyS0).
//   role: Application role {"tx", "rx"}.
//   baudrate: Baudrate of the serial port.
//   nTries: Maximum number of frame retries.
//   timeout: Frame timeout.
//   filename: Name of the file to send / receive.
void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename);

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>

#endif // _APPLICATION_LAYER_H_
