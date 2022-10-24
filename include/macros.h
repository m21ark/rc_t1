#ifndef _MACROS_DEFINES_H_
#define _MACROS_DEFINES_H_

// =========== SERIAL PORT =============

#define BAUDRATE 38400  // NOTA: AQUI não é suposto meter o B ... a conversão é feita noutro lado
#define _POSIX_SOURCE 1 // POSIX compliant source

// =========== MAIN =============

#define TIMEOUT 4
#define N_TRIES 3

// =========== MISC =============

#define FALSE 0
#define TRUE 1

// =========== DEBUG =============

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif
// define SLOW_SEND to make transmiter wait 1s between packet sending

#define UNUSED(x) (void)(x)

#endif