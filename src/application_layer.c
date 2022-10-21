#include "application_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    // Set intial values
    LinkLayerRole LRole = (!strcmp(role, "tx")) ? LlTx : LlRx;
    LinkLayer connectionParameters = {"", LRole, baudRate, nTries, timeout};
    strcpy(connectionParameters.serialPort, serialPort);

    printf("\n\nStart of program.\nAttempting to create connection...\n");

    // Open connection between TX and RX
    if (llopen(connectionParameters) < 0)
    {
        printf("Connection couldn't be established.\n");
        return;
    }

    printf("\nA connection was established.\n");

    if (connectionParameters.role == LlTx)
        if (sendFile(filename) < 0)
        {
            printf("File sending failed.\n");
            return;
        }

    if (connectionParameters.role == LlRx)
        if (rcvFile(filename) < 0)
        {
            printf("File receiving failed.\n");
            return;
        }

    printf("File transfer complete. Starting to close connection...\n");

    if (llclose(1) < 0)
    {
        printf("The closing of the connection failed.\n");
        return;
    }
    else
    {
        printf("Connection closed.\n");
    }

    printf("End of program.\n");
}
