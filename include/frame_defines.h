#ifndef _FRAME_DEFINES_H_
#define _FRAME_DEFINES_H_


#define FLAG 0x7E

#define SET 0x03
#define UA 0x07
#define CTRL_S(s) ((s) << 6) // VER SE ESTÁ CORRETO 


#define ADDR_ER 0x03

#define BCC(addr, ctrl) (addr^ctrl)

#endif
