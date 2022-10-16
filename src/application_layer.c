// Application layer protocol implementation

#include "application_layer.h"

FILE *fp_tx = NULL;
FILE *fp_rx = NULL;

void al_close_rx()
{
    fclose(fp_rx);
}

void al_close_tx()
{
    fclose(fp_tx);
}

int al_open_tx(const char *filename_tx)
{
    fp_tx = fopen(filename_tx, "r");
    fseek(fp_tx, 0, SEEK_END);
    int file_size = ftell(fp_tx);
    fseek(fp_tx, 0, SEEK_SET);
    return file_size;
}

void al_open_rx(const char *filename_rx)
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

    LinkLayerRole LRole = (!strcmp(role, "tx")) ? LlTx : LlRx;
    LinkLayer connectionParameters = {"", LRole, baudRate, nTries, timeout};

    strcpy(connectionParameters.serialPort, serialPort);

    if (llopen(connectionParameters) < 0)
    {
        printf("Connection was not possible");
        return;
    }

    if (connectionParameters.role == LlTx)
    {
        int file_send_size = al_open_tx(filename);
        UNUSED(file_send_size);
        unsigned char message_send[AL_DATA_SIZE];

        int num_bytes_send;
        while (num_bytes_send = al_write(message_send, AL_DATA_SIZE))
            llwrite(message_send, AL_DATA_SIZE);
    }

    if (connectionParameters.role == LlRx)
    {

        al_open_rx(filename);
        unsigned char message_rcv[AL_DATA_SIZE];
x
        // unsigned char *aux = "ola meu deus\0";
        // al_read(aux, strlen(aux));
        // exit(0);

        int i = 20; // falta a parte de controlar o tamanho do ficheiro para o receiver saber o q esperar
        while (i--)
        {
            llread(message_rcv);
            printf("\nWRITING=|%s|\n", message_rcv);
            al_read(message_rcv, AL_DATA_SIZE);
        }
        al_close_rx();
    }

    llclose(1);
}
