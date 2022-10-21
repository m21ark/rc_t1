#ifndef _FRAME_DEFINES_H_
#define _FRAME_DEFINES_H_

// State Machine

#define EXIT_SET_STATE stop
#define ENTRY_SET_STATE start

#define BUF_SIZE 5000

// End of state machine

#define FLAG 0x7E

#define SET 0x03
#define UA 0x07
#define CTRL_S(s) ((s) << 6) // VER SE ESTÁ CORRETO
#define DISC 0xbC
#define RR(n) (((n) << 7) | 0x05)
#define REJ(n) (((n) << 7) | 0x01)

#define ADDR_ER 0x03

#define BCC(addr, ctrl) (addr ^ ctrl)

#define UNUSED(x) (void)(x)

#define ESC 0x7d
#define XOR_FLAG 0x5e
#define XOR_ESC 0x5d

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// MISC
#define FALSE 0
#define TRUE 1

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

// APPLICATION LAYER DEFINES

#define N_TRIES 3
#define TIMEOUT 4

// 60 64
// 256 252
#define MAXSIZE_DATA 250  // Max size of AL Data Packet
#define MAXSIZE_FRAME 504 // Max size of Individual Frame
// FRAME > DATA ? MAXSIZE_FRAME = MAXSIZE_DATA + 4

#define SEQUENCE_MODULO 16 // packet sending rotation to know if out of order

#define CTRL_DATA 0x01
#define CTRL_START 0x02
#define CTRL_END 0x03

#define TYPE_FILESIZE 0x00
#define TYPE_FILENAME 0x01
#define MAXSIZE_FILE_NAME 127

// Debug

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#endif // _FRAME_DEFINES_H_