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

    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 1; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    // printf("New termios structure set\n");

    if (connectionParameters.role == LlRx)
    {
        if (readMessageWithResponse(fd) < 0)
        {
            return -1;
        }
        set_rx_ready();
    }

    if (connectionParameters.role == LlTx)
    {
        unsigned char cmd[5] = {FLAG, ADDR_ER, SET, BCC(ADDR_ER, SET), FLAG};

        if (sendAndWaitMessage(fd, cmd, 5) < 0)
        {
            return -1;
        }
        set_tx_ready();
    }

    return 1;
}

int llwrite(const unsigned char *buf, int bufSize)
{
    static int w_packet = 0;

    int ret;
    int numTries = 0;

    do
    { // DISCUTIR COM O STOR ::: SE RECEBER REJECT DEVO REPETIR 3 VEZES OU ISSO JÁ CONTA COMO UMA E SO TENHO MAIS DUAS TRANS. ?
        numTries++;
        ret = sendInformationFrame(fd, buf, bufSize, w_packet);

        printf("\n--------------------------------------|%d|%d\n", ret, numTries);
        if (ret == 0)
        {
            w_packet = (w_packet + 1) % 2;
            printf("\nW_PACKET ::: %d\n", w_packet);
            return 0; // TODO: para quê retornar o numero de bytes escritos ?
        }
        else if (ret < 0)
        {
            // WE already waited 12 seconds
            break;
        }

    } while (numTries < 3);

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
        printf("\nRET = 0 ::: %d\n", r_packet);

        ///sleep(4);
        unsigned char cmd[5] = {FLAG, ADDR_ER, RR(r_packet), BCC(ADDR_ER, RR(r_packet)), FLAG};
        write(fd, cmd, 5);

        return get_data(packet);
    }
    else if (r < 0)
    {
        unsigned char cmd[5] = {FLAG, ADDR_ER, REJ(r_packet), BCC(ADDR_ER, REJ(r_packet)), FLAG};
        write(fd, cmd, 5);
        printf("Burros2\n");
        printf("\nREJ\n");
        return -1;
    }

    printf("LL read return 0\n");
    return 0;
}

int llclose(int showStatistics)
{
    if (is_tx())
    {
        printf("\nTX\n");
        unsigned char cmd[5] = {FLAG, ADDR_ER, DISC, BCC(ADDR_ER, DISC), FLAG};

        if (sendAndWaitMessage(fd, cmd, 5) < 0)
        {
            return -1;
        }

        unsigned char ua_cmd[5] = {FLAG, ADDR_ER, UA, BCC(ADDR_ER, UA), FLAG};
        write(fd, ua_cmd, 5);
    }
    else if (is_rx())
    {
        printf("\nRX\n");

        int r = readMessageWithResponse(fd);

        if (r < 0)
        {
            return -1;
        }
    }

    UNUSED(showStatistics);
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}
