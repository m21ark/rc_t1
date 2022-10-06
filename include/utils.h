#ifndef _RCOM_UTILS_H
#define _RCOM_UTILS_H

#include "frame_defines.h"
#include "stdio.h"

int countProblematicFlags(unsigned char * data, int dataSize);

int stuffData(unsigned char * data, int dataSize, unsigned char * stData, int stSize);


#endif
