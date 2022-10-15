// Link layer protocol implementation

#include "link_layer.h"

extern int (*set_state[])(unsigned char c);

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    // TODO
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

    printf("New termios structure set\n");

    if (connectionParameters.role == LlRx)
    {
        readMessageWithResponse(fd);
    }

    if (connectionParameters.role == LlTx)
    {
        unsigned char cmd[5] = {FLAG, ADDR_ER, SET, BCC(ADDR_ER, SET), FLAG};

        if (sendAndWaitMessage(fd, cmd, 5) < 0)
        {
            return -1;
        }
    }

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    static int w_packet = 0;

    int ret;
    int numTries = 0;

    do
    { // DISCUTIR COM O STOR ::: SE RECEBER REJECT DEVO REPETIR 3 VEZES OU ISSO JÁ CONTA COMO UMA E SO TENHO MAIS DUAS TRANS. ?
        numTries++;
        ret = sendInformationFrame(fd, buf, bufSize, w_packet);

        printf("\n---------------------------------------%d\n", ret);
        if (ret == 0)
        {
            w_packet = (w_packet + 1) % 2;
            return 0; // TODO: para quê retornar o numero de bytes escritos ?
        }
        if (ret < 0)
        {
            // WE already waited 12 seconds
            break;
        }

    } while (numTries < 3);

    return -1;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    static int r_packet = 0;

    int r = readMessageWithResponse(fd);

    if (r > 0)
    {
        r_packet = (r_packet + 1) % 2;
        unsigned char cmd[5] = {FLAG, ADDR_ER, RR(r_packet), BCC(ADDR_ER, RR(r_packet)), FLAG};
        write(fd, cmd, 5);

        // parte de devolver info lida no packet passado em arg
        int data_size = get_data_size();
        unsigned char sms[data_size];
        get_data(sms);
        memcpy(packet, sms, data_size);
        // printf("\nTO USE = |%s|%d|\n", sms, data_size);
    }
    else if (r < 0)
    {
        unsigned char cmd[5] = {FLAG, ADDR_ER, REJ(r_packet), BCC(ADDR_ER, REJ(r_packet)), FLAG};
        write(fd, cmd, 5);
        printf("\nREJ\n");
    }

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO
    // Restore the old port settings
    UNUSED(showStatistics);
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}
