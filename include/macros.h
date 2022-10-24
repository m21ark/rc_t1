#ifndef _FRAME_DEFINES_H_
#define _FRAME_DEFINES_H_

// =========== SERIAL PORT =============

#define BAUDRATE 38400 // NOTA: AQUI não é suposto meter o B ... a conversão é feita noutro lado
#define _POSIX_SOURCE 1 // POSIX compliant source

// =========== MAIN =============

#define TIMEOUT 4
#define N_TRIES 3

// =========== STATE MACHINE =============

#define EXIT_SET_STATE stop
#define ENTRY_SET_STATE start
#define BUF_SIZE 5000

// =========== PACKETS =============

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

// =========== SPECIAL FLAGS =============

#define ESC 0x7d
#define XOR_FLAG 0x5e
#define XOR_ESC 0x5d

// =========== MISC =============

#define FALSE 0
#define TRUE 1

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

// =========== ALARM =============

#define TURN_OFF_ALARM alarm(0);
#define SET_ALARM_TIME(x) alarm(x);

#define MAX_IDLE_TIME 12
#define REATTEMPT_WAIT_TIME 3

// =========== DEBUG =============

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif
// define SLOW_SEND to make transmiter wait 1s between packet sending

#endif // _FRAME_DEFINES_H_