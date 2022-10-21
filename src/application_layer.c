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

int readFromFile(unsigned char *message_send, unsigned msg_size)
{
    return fread(message_send, 1, msg_size, fp_tx);
}

int writeToFile(unsigned char *message_rcv, unsigned msg_size)
{
    return fwrite(message_rcv, msg_size, 1, fp_rx);
}

int makeCtrlPacket(unsigned char ctrlByte, unsigned char *packet, const char *filename, int filesize)
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

    int length;

    if (packetBuffer[1] == TYPE_FILESIZE)
    {

        *fileSize = 0;
        length = (int)packetBuffer[2];
        for (int i = 0; i < length; i++)
            *fileSize = *fileSize * 256 + (int)packetBuffer[3 + i];
    }
    else
    {
        printf("Error during command packet filesize parsing.\n");
        return -1;
    }

    int fileNameStart = 5 + length;

    if (packetBuffer[fileNameStart - 2] == TYPE_FILENAME)
    {
        length = (int)packetBuffer[fileNameStart - 1];
        for (int i = 0; i < length; i++)
            fileName[i] = packetBuffer[fileNameStart + i];
    }
    else
    {
        printf("Error during command packet filename parsing.\n");
        return -1;
    }

    return 0;
}

int makeDataPacket(unsigned char *packet, int seqNum, unsigned char *data, int dataLen)
{

    int l1 = dataLen % 256;
    int l2 = dataLen / 256;

    packet[0] = CTRL_DATA;
    packet[1] = seqNum;
    packet[2] = l2;
    packet[3] = l1;

    // actual data packets
    for (int i = 0; i < dataLen; i++)
        packet[i + 4] = data[i];

    return dataLen + 4;
}

int parseDataPacket(unsigned char *packet, unsigned char *data)
{

    if (packet[0] != CTRL_DATA)
        return -1;

    int l1 = packet[3], l2 = packet[2];
    int data_size = 256 * l2 + l1;

    // actual data packets
    for (int i = 0; i < data_size; i++)
        data[i] = packet[i + 4];

    return packet[1];
}

int sendFile(const char *filename)
{

    printf("Opening file to be sent...\n");
    int file_send_size = al_open_tx(filename);

    unsigned char message_send[MAXSIZE_FRAME];
    unsigned char data[MAXSIZE_DATA];

    printf("Sending Start Command Packet...\n");
    int packet_size = makeCtrlPacket(CTRL_START, message_send, filename, file_send_size);
    if (llwrite(message_send, packet_size) < 0)
    {
        printf("Unable to send Start Command Packet.\n");
        return -1;
    }

    printf("Sending Main File...\n");
    int seqNum = 0, num_read_bytes;
    while ((num_read_bytes = readFromFile(data, MAXSIZE_DATA)))
    {

        int msg_size = makeDataPacket(message_send, seqNum, data, num_read_bytes);

        if (llwrite(message_send, msg_size) < 0)
            return -1;
        else
            printf("\nAQUI ESTA UMA INVOCAÇAO\n");

        seqNum = (seqNum + 1) % SEQUENCE_MODULO;
    }

    printf("Main file was sent.\nSending End Command Packet...\n");

    // Send End Command packet
    packet_size = makeCtrlPacket(CTRL_END, message_send, filename, file_send_size);

    if (llwrite(message_send, packet_size) < 0)
    {
        printf("Unable to send END Command Packet.\n");
        return -1;
    }

    al_close_tx();
    return 0;
}

int rcvFile(const char *filename)
{

    printf("Waiting for Start Command Packet...\n");

    unsigned char message_rcv[MAXSIZE_FRAME];
    unsigned char data[MAXSIZE_DATA];

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
    int packet_size, num_bytes_rcv = 0, seqNum = 0;
    while (1)
    {
        packet_size = llread(message_rcv);

        if (packet_size <= 0)
        {
            printf("REJ DATA!%d!\n", packet_size);
            continue;
        }

        if (message_rcv[0] == CTRL_END)
            break;

        else if (message_rcv[0] == CTRL_DATA)
        {
            int rcv_seqNum = parseDataPacket(message_rcv, data);
            printf("PACKET: %d\n", packet_size);
            printf("PACKET NR : %d", rcv_seqNum);
            if (seqNum != rcv_seqNum)
            {
                printf("Received packet out of order!\n Expected %d and recieved %d\n", seqNum, rcv_seqNum);
                return -1;
            }

            seqNum = (seqNum + 1) % SEQUENCE_MODULO;

            int data_size = packet_size - 4; // removing the 4 bytes for the data packet head
            writeToFile(data, data_size);    // writing data bytes to file

            num_bytes_rcv += data_size;
            printf("Current progress: %d/%d\n", num_bytes_rcv, file_rcv_size);

            if (num_bytes_rcv == file_rcv_size)
                break; // File is complete
        }

        printf("\nPACKET_NR:: %d\n", seqNum);
        // else
        //{
        //     printf("Received a packet without the data flag.\n");
        //     return -1;
        // }
    }

    printf("Write to file complete.\nWaiting for End Control Packet\n");

    // Receive End Command Packet
    llread(message_rcv);
    if (message_rcv[0] != CTRL_END)
    {
        printf("Expected End Control Packet but got none.\n");
        printf("CTRL %d", message_rcv[0]);
        return -1;
    }

    int file_rcv_size_end;
    char rcv_fileName_end[MAXSIZE_FILE_NAME];

    if (parseCtrlPacket(message_rcv, &file_rcv_size_end, rcv_fileName_end) < 0)
    {
        printf("Error parsing End Command packet.\n");
        return -1;
    }

    if (strcmp(rcv_filename, rcv_fileName_end) != 0)
    {
        printf("\nFilenames of start/end control packet dont match: ");
        printf("|%s| != |%s|\n", rcv_filename, rcv_fileName_end);
        return -1;
    }

    if (file_rcv_size != file_rcv_size_end)
    {
        printf("\nFilesize of start/end control packet dont match: ");
        printf("|%d| != |%d|\n", file_rcv_size, file_rcv_size_end);
        return -1;
    }

    al_close_rx();
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
