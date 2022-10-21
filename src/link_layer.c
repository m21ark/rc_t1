#include "link_layer.h"

extern int (*set_state[])(unsigned char c);

int llopen(LinkLayer connectionParameters)
{
    signal(SIGALRM, alarm_handler);

    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    struct termios newtio;

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1;
    newtio.c_cc[VMIN] = 0;

    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    // Set up the connection between Tx and Rx

    if (connectionParameters.role == LlRx)
    {
        if (readMessageWithResponse(fd) < 0)
        {
            DEBUG_PRINT("Connection attempt to rx failed\n");
            return -1;
        }

        set_rx_ready();
    }

    if (connectionParameters.role == LlTx)
    {
        unsigned char cmd[5] = {FLAG, ADDR_ER, SET, BCC(ADDR_ER, SET), FLAG};

        if (sendAndWaitMessage(fd, cmd, 5) < 0)
        {
            DEBUG_PRINT("Connection attempt to tx failed\n");
            return -1;
        }

        set_tx_ready();
    }

    return 1; // shouldnt be zero?
}

int llwrite(const unsigned char *buf, int bufSize)
{
    static int w_packet = 0;
    int numTries = 0, ret;

    do
    {
        numTries++;
        ret = sendInformationFrame(fd, buf, bufSize, w_packet);

        DEBUG_PRINT("\n--------------------------------------|%d|%d\n", ret, numTries);
        if (ret == 0)
        {
            w_packet = (w_packet + 1) % 2;
            DEBUG_PRINT("\nW_PACKET ::: %d\n", w_packet);
            return 0; // TODO: para quê retornar o numero de bytes escritos ?
        }
        else if (ret < 0)
        {
            // WE already waited 12 seconds
            DEBUG_PRINT("Already waited 12s. Stop trying to send msg.\n");
            break;
        }

    } while (numTries < 3);

    DEBUG_PRINT("llwrite returned -1\n");
    return -1;
}

int llread(unsigned char *packet)
{
    static int r_packet = 0;
    set_rcv_packet_nr(r_packet);
    int r = readMessageWithResponse(fd);

    if (r > 0) // if rcv occured in right order
    {
        r_packet = (r_packet + 1) % 2;
        DEBUG_PRINT("readMessage > 0 with r_packet= %d\n", r_packet);

        unsigned char cmd[5] = {FLAG, ADDR_ER, RR(r_packet), BCC(ADDR_ER, RR(r_packet)), FLAG};
        write(fd, cmd, 5);

        DEBUG_PRINT("Returning data from packet\n");
        return get_data(packet);
    }
    else if (r < 0)
    {
        unsigned char cmd[5] = {FLAG, ADDR_ER, REJ(r_packet), BCC(ADDR_ER, REJ(r_packet)), FLAG};
        write(fd, cmd, 5);
        DEBUG_PRINT("REJ was sent\n");
        return -1;
    }

    DEBUG_PRINT("LL read return 0\n");
    return 0;
}

int llclose(int showStatistics)
{
    UNUSED(showStatistics); // TODO -> Fazer a analise dos dados

    // Transmitter closing
    if (is_tx())
    {
        DEBUG_PRINT("TX Closing...\n");
        unsigned char cmd[5] = {FLAG, ADDR_ER, DISC, BCC(ADDR_ER, DISC), FLAG};

        if (sendAndWaitMessage(fd, cmd, 5) < 0)
        {
            DEBUG_PRINT("llclose tx await for response failed");
            return -1;
        }

        DEBUG_PRINT("Tx sending final UA to close\n");
        unsigned char ua_cmd[5] = {FLAG, ADDR_ER, UA, BCC(ADDR_ER, UA), FLAG};
        write(fd, ua_cmd, 5);
        sleep(1);
    }

    // Receiver closing
    if (is_rx())
    {
        DEBUG_PRINT("RX Closing...\n");
        int r = readMessageWithResponse(fd);

        if (r < 0)
        {
            DEBUG_PRINT("Rx llclose read message failed\n");
            return -1;
        }
    }

    // Recover old serial port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    DEBUG_PRINT("closing file descriptor\n");
    close(fd);
    return 0;
}
