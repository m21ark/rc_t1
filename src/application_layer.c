// Application layer protocol implementation

#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    LinkLayerRole LRole = (!strcmp(role, "tx")) ? LlTx : LlRx;
    LinkLayer connectionParameters = {"", LRole, baudRate, nTries, timeout};

    strcpy(connectionParameters.serialPort, serialPort);

    llopen(connectionParameters);

    if (connectionParameters.role == LlTx)
    {
        unsigned char data[2] = {FLAG, ESC};
        llwrite(data, 2);
    }

    return 0;
}
