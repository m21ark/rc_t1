// Application layer protocol implementation

#include "application_layer.h"

extern int (*set_state[])(unsigned char c);

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPort, O_RDWR | O_NOCTTY);
    

    if (fd < 0)
    {
        perror(serialPort);
        exit(-1);
    }

    struct termios oldtio;
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

    if (strcmp(role, "rx") == 0)
    {

        unsigned int STOP = FALSE;
        unsigned char buf = 0;
        unsigned char bytes;
        while (1) // SEE THIS LATTER 
        {
            bytes = read(fd, &buf, 1);
            enum set_state_codes st = get_set_state();
            set_state_fun = set_state[st];
            enum set_ret_codes rt = set_state_fun(buf);

            printf("rt:%d\n", rt);
            set_set_state(set_lookup_transitions(st, rt));

            printf("state:%d:%d\n", get_set_state(), buf);
            
            if (get_set_state() == EXIT_SET_STATE)
            {
                printf("SET RECIEVED");
                break;
            }
        }

       unsigned char cmd[5] = {FLAG, ADDR_ER, UA, BCC(ADDR_ER, UA), FLAG};
       int j = write(fd, cmd, 5);
       printf("%d bytes written\n", j);
       
    }

    if (strcmp(role, "tx") == 0)
    {
        unsigned char cmd[5] = {FLAG, ADDR_ER, SET, BCC(ADDR_ER, SET), FLAG};

        sendAndWaitMessage(fd, cmd, 5);
        
    }

    sleep(1);

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}
