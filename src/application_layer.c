// Application layer protocol implementation

#include "application_layer.h"

FILE *fp_tx = NULL;
FILE *fp_rx = NULL;
int file_send_size = 0;

int al_open_tx(char *filename_tx)
{
    fp_tx = fopen(filename_tx, "r");
    fseek(fp_tx, 0, SEEK_END);
    int file_size = ftell(fp_tx);
    fseek(fp_tx, 0, SEEK_SET);
    return file_size;
}

void al_open_rx(char *filename_rx)
{
    fp_rx = fopen(filename_rx, "w");
}

int al_write(char *message_send, unsigned msg_size)
{
    return fread(message_send, 1, msg_size, fp_tx);
}

int al_read(char *message_rcv, unsigned msg_size)
{
    return fwrite(message_rcv, msg_size, 1, fp_rx);
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    file_send_size = al_open_tx("teste1.txt");
    al_open_rx("teste2.txt");

    /*
    char message_send[AL_DATA_SIZE + 1];

    int num_bytes_sent;
    while (num_bytes_sent = al_write(message_send, AL_DATA_SIZE))
        al_read(message_send, num_bytes_sent);

    return;
    */

    UNUSED(filename);
    LinkLayerRole LRole = (!strcmp(role, "tx")) ? LlTx : LlRx;
    LinkLayer connectionParameters = {"", LRole, baudRate, nTries, timeout};

    strcpy(connectionParameters.serialPort, serialPort);

    if (llopen(connectionParameters) < 0)
    {
        printf("Connection was not possible");
        return;
    }
    unsigned char packet;

    if (connectionParameters.role == LlTx)
    {
        unsigned char message_send[AL_DATA_SIZE];

        int num_bytes_send;
        while (num_bytes_send = al_write(message_send, AL_DATA_SIZE))
            llwrite(message_send, AL_DATA_SIZE);
    }

    if (connectionParameters.role == LlRx)
    {
        // temporary cheatcode to inform receiver of what they need to read
        int i = 1 + file_send_size / AL_DATA_SIZE + ((file_send_size % AL_DATA_SIZE != 0) ? 1 : 0);
        printf("\n&%d&\n", i);
        while (i--)
        {
            llread(&packet);
            printf("#%d#", i);
        }
    }

    llclose(1);
}
