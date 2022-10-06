#include "../include/utils.h"


int countProblematicFlags(unsigned char * data, int dataSize) {
    int n_misleading_flags = 0;
    for (int i = 0; i < dataSize; i++)
    {
        if (data[i] == FLAG || data[i] == ESC)
        {
            n_misleading_flags++;
        }
    }
    return n_misleading_flags - 2;
}

int stuffData(unsigned char * data, int dataSize, unsigned char * stData, int stSize) {
    
    stData[0] = FLAG;
    stData[stSize - 1] = FLAG;
    for (int i = 1, j = 1; i < dataSize - 1; i++, j++)
    {
        if (data[i] == FLAG)
        {
            printf("FLAG %d,%d", i, j);
            stData[j++] = ESC;
            stData[j] = XOR_FLAG;
        }
        else if (data[i] == ESC) 
        {
            printf("ESC %d,%d", i, j);

            stData[j++] = ESC;
            stData[j] = XOR_ESC;
        }
        else {
            stData[j] = data[i];
        }
    }

    return 0;
}
