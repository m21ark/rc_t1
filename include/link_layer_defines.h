#ifndef _LL_DEFINES_H_
#define _LL_DEFINES_H_

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

// =========== SPECIAL FLAGS =============

#define ESC 0x7d
#define XOR_FLAG 0x5e
#define XOR_ESC 0x5d

#endif
