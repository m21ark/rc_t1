#ifndef _APP_LAYER_DEFINES_H_
#define _APP_LAYER_DEFINES_H_

// =========== APPLICATION LAYER =============

#define MAXSIZE_FRAME 504                // Max size of Individual Frame
#define MAXSIZE_DATA (MAXSIZE_FRAME - 4) // Max size of AL Data Packet
#define SEQUENCE_MODULO 16               // packet sending rotation to know if out of order

#define CTRL_DATA 0x01
#define CTRL_START 0x02
#define CTRL_END 0x03

#define TYPE_FILESIZE 0x00
#define TYPE_FILENAME 0x01
#define MAXSIZE_FILE_NAME 127

#endif