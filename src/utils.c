#include "../include/utils.h"


int countProblematicFlags(unsigned char * data, int dataSize) {
    int n_misleading_flags = 0;
    for (int i = 0; i < dataSize; i++)
    {
        if (data[i] == FLAG)
        {
            n_misleading_flags++;
        }
    }
    return n_misleading_flags -2;
}

int stuffData(unsigned char * data, int dataSize, unsigned char * stData, int stSize) {
    
    stData[0] = FLAG;
    stData[stSize] = FLAG;
    for (int i = 1; i < dataSize - 1; i++)
    {
        if (data[i] == FLAG)
        {
            stData[i++] = ESC;
            stData[i] = XOR_FLAG;
        }
        else if (data[i] == ESC) 
        {
            stData[i++] = ESC;
            stData[i] == XOR_ESC;
        }
    }

    return 0;
}

