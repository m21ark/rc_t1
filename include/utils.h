#ifndef _RCOM_UTILS_H
#define _RCOM_UTILS_H

#include <stdio.h>
#include "macros.h"

int countProblematicFlags(unsigned char *data, int dataSize);

int stuffData(unsigned char *data, int dataSize, unsigned char *stData, int stSize);

int unstuffData(unsigned char *data, int dataSize, unsigned char *stData);

unsigned char BCC2(const unsigned char *data, int dataSize);

#endif // _RCOM_UTILS_H