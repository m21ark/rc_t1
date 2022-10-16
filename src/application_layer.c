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

int readFromFile(char *message_send, unsigned msg_size)
{
    return fread(message_send, 1, msg_size, fp_tx);
}

int writeToFile(char *message_rcv, unsigned msg_size)
{
    return fwrite(message_rcv, msg_size, 1, fp_rx);
}

int makeCtrlPacket(unsigned char ctrlByte, unsigned char *packet, char *filename, int filesize)
{

    packet[0] = ctrlByte;
    packet[1] = TYPE_FILESIZE;

    int length = 0;
    int currentFileSize = filesize;

    // cicle to separate file size (v1) in bytes
    while (currentFileSize > 0)
    {
        int rest = currentFileSize % 256;
        int div = currentFileSize / 256;
        length++;

        // shifts all bytes to the right, to make space for the new byte
        for (unsigned int i = 2 + length; i > 3; i--)
            packet[i] = packet[i - 1];

        packet[3] = (unsigned char)rest;
        currentFileSize = div;
    }

    packet[2] = (unsigned char)length;
    packet[3 + length] = TYPE_FILENAME;

    int fileNameStart = 5 + length; // beginning of v2

    packet[4 + length] = (unsigned char)(strlen(filename) + 1); // adds file name length (including '\0)

    for (unsigned int j = 0; j < (strlen(filename) + 1); j++)
        packet[fileNameStart + j] = filename[j]; // strlen(fileName) + 1 in order to add the '\0' char

    return 3 + length + 2 + strlen(filename) + 1; // total length of the packet
}

int parseCtrlPacket(unsigned char *packetBuffer, int *fileSize, char *fileName)
{

    if (packetBuffer[0] != CTRL_START && packetBuffer[0] != CTRL_END)
    {
        printf("Packet being parsed doesn't correspond to Command packet.\n");
        return -1;
    }

    int length1;

    if (packetBuffer[1] == TYPE_FILESIZE)
    {

        *fileSize = 0;
        length1 = (int)packetBuffer[2];

        for (int i = 0; i < length1; i++)
            *fileSize = *fileSize * 256 + (int)packetBuffer[3 + i];
    }
    else
    {
        printf("Error during command packet filesize parsing.\n");
        return -1;
    }

    int length2;
    int fileNameStart = 5 + length1;

    if (packetBuffer[fileNameStart - 2] == TYPE_FILENAME)
    {
        length2 = (int)packetBuffer[fileNameStart - 1];

        for (int i = 0; i < length2; i++)
            fileName[i] = packetBuffer[fileNameStart + i];
    }
    else
    {
        printf("Error during command packet filename parsing.\n");
        return -1;
    }

    return 0;
}

int sendFile(char *filename)
{

    // open file to send
    printf("Opening file to be sent...\n");
    int file_send_size = al_open_tx(filename);
    UNUSED(file_send_size);
    unsigned char message_send[AL_DATA_SIZE];

    printf("Sending Start Command Packet...\n");
    // Send the Start Command packet
    int packet_size = makeCtrlPacket(CTRL_START, message_send, filename, file_send_size);
    if (llwrite(message_send, packet_size) < 0)
    {
        printf("Unable to send Start Command Packet.\n");
        return -1;
    }

    printf("Sending Main File...\n");
    // send main file content
    int num_bytes_send;
    while (num_bytes_send = readFromFile(message_send, AL_DATA_SIZE))
        if (llwrite(message_send, AL_DATA_SIZE) < 0)
            break;

    printf("Main file was sent.\nSending End Command Packet...\n");

    // Send End Command packet
    packet_size = makeCtrlPacket(CTRL_START, message_send, filename, file_send_size);
    llwrite(message_send, packet_size);

    al_close_tx();
    return 0;
}

int rcvFile(char *filename)
{

    printf("Waiting for Start Command Packet...\n");
    unsigned char message_rcv[AL_DATA_SIZE];
    llread(message_rcv);

    if (message_rcv[0] != CTRL_START)
    {
        printf("Expected Start Control Packet but got none.\n");
        return -1;
    }

    int file_rcv_size;
    char rcv_filename[MAXSIZE_FILE_NAME];

    if (parseCtrlPacket(message_rcv, &file_rcv_size, rcv_filename) < 0)
    {
        printf("Error parsing Start Command packet.\n");
        return -1;
    }

    // create file where to write incoming contents
    printf("Receiving file info: file '%s' with size %dB\n", rcv_filename, file_rcv_size);
    al_open_rx(filename);

    printf("Starting to write to file...\n");
    int packet_size, num_bytes_rcv = 0;
    while (1)
    {
        packet_size = llread(message_rcv);
        writeToFile(message_rcv, AL_DATA_SIZE);

        num_bytes_rcv += packet_size * 8;
        printf("Current progress: %d/%d\n", num_bytes_rcv, file_rcv_size);

        if (num_bytes_rcv > file_rcv_size)
            break; // File is complete
    }

    printf("Write to file complete.\nWaiting for End Control Packet\n");

    // Receive End Command Packet
    llread(message_rcv);
    if (message_rcv[0] != CTRL_END)
    {
        printf("Expected End Control Packet but got none.\n");
        return -1;
    }

    int file_rcv_size_end;
    char rcv_fileName_end[MAXSIZE_FILE_NAME];
    if (parseCtrlPacket(message_rcv, &file_rcv_size, filename) < 0)
    {
        printf("Error parsing End Command packet.\n");
        return -1;
    }

    if (strcmp(rcv_filename, rcv_fileName_end) != 0)
    {
        printf("Filenames of start/end control packet dont match.\n");
        return -1;
    }

    if (file_rcv_size != file_rcv_size_end)
    {
        printf("Filesize of start/end control packet dont match.\n");
        return -1;
    }

    al_close_rx();
    printf("END OF RCV_FILE\n");
    return 0;
}

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

    printf("End of program.\n");
}
