#include "../include/message.h"

extern int (*set_state[])(unsigned char c);
static int (*set_state_fun)(unsigned char c);

int alarm_flag = 0;

void alarm_handler()
{
    alarm_flag = 1;
}

int sendAndWaitMessage(int fd, unsigned char *msg, int messageSize)
{
    int numTries = 0;
    int ret;

    (void)signal(SIGALRM, alarm_handler); // TODO : NOT HERE

    do
    {
        numTries++;
        alarm_flag = 0;

        ret = write(fd, msg, messageSize);
        printf("\nConnection attempt number %d\n", numTries);
        alarm(3);

        unsigned char buf = 0;
        unsigned char bytes;

        while (!alarm_flag)
        {
            bytes = read(fd, &buf, 1);
            if (bytes == 0)
                continue;

            enum set_state_codes st = get_set_state();
            set_state_fun = set_state[st];
            enum set_ret_codes rt = set_state_fun(buf);

            printf("rt:%d\n", rt);
            set_set_state(set_lookup_transitions(st, rt));

            printf("state:%d:%d:%c\n", get_set_state(), buf, buf);

            if (get_set_state() == EXIT_SET_STATE)
            {
                printf("UA/RR/REJ/DISC RECIEVED\n");
                // IF REJ IS RECIEVED WE DONT WANT TO WAIT 3 seconds ... that's why we directly go to the final state
                break;
            }
        }

    } while (numTries < 3 && get_set_state() != EXIT_SET_STATE);

    alarm(0);

    if (get_set_state() != EXIT_SET_STATE)
    {
        printf("\nFailed to get a response.\n");
        ret = -1;
    }

    set_set_state(ENTRY_SET_STATE);

    return ret;
}

int sendInformationFrame(int fd, const unsigned char *data, int dataSize, int packet)
{ // maybe make a struct

    unsigned char cmd[dataSize + 6]; // acho que depende do compilador VER

    cmd[0] = FLAG;
    cmd[1] = ADDR_ER;
    cmd[2] = CTRL_S(packet);
    cmd[3] = BCC(cmd[1], cmd[2]);

    memcpy(cmd + 4, data, dataSize);
    // TODO: make a function(done) and ask teacher if we can use other types of controlling
    unsigned char bcc2 = BCC2(data, dataSize);

    cmd[dataSize + 4] = bcc2;
    cmd[dataSize + 5] = FLAG;

    int n_mis_flags = countProblematicFlags(cmd, dataSize + 6);

    unsigned char stuffed_cmd[dataSize + 6 + n_mis_flags];

    stuffData(cmd, dataSize + 6, stuffed_cmd, dataSize + 6 + n_mis_flags);

    int ret = sendAndWaitMessage(fd, stuffed_cmd, dataSize + 6 + n_mis_flags);

    unsigned char c = get_control();
    if (ret > 0 && ((packet == 0 && c == RR(1)) || (packet == 1 && c == RR(0))))
    {
        return 0;
    }

    if (ret < 0)
    {
        return -1; // The number of restranmissions was exceeded
    }

    return 1; // THIS IS THE CASE IN WHICH A REJ WAS RECEIVED
}

int readMessageWithResponse(int fd)
{
    unsigned char buf = 0;
    unsigned char bytes;

    alarm(12); // TODO MAKE A MACRO
    alarm_flag = 0;

    while (!alarm_flag) // SEE THIS LATTER :: É importante uma vez que assim fica a ler lixo quando à barulho
    {
        bytes = read(fd, &buf, 1);
        if (bytes == 0)
            continue; // TODO :: Maybe put a Timerout to stop the app
        enum set_state_codes st = get_set_state();
        set_state_fun = set_state[st];
        enum set_ret_codes rt = set_state_fun(buf);

        if (rt == BCC2_NOT_OK)
        {
            return -1;
        }

        printf("rt:%d | ", rt);
        set_set_state(set_lookup_transitions(st, rt));

        if (get_set_state() != 6)
            printf(" state:%d:%d:%c\n", get_set_state(), buf, buf);
        else
            printf(" message>|%c|\n", buf);

        if (get_set_state() == EXIT_SET_STATE)
        {
            printf("SET RECIEVED\n");
            set_set_state(ENTRY_SET_STATE);
            alarm(0);
            if (get_control() == SET)
            {
                unsigned char cmd[5] = {FLAG, ADDR_ER, UA, BCC(ADDR_ER, UA), FLAG};
                write(fd, cmd, 5);
            }
            else if (get_control() == CTRL_S(0) || get_control() == CTRL_S(1))
            {
                return get_data_size();
            }
            else if (get_control() == DISC)
            {
                printf("\nDISC\n");
                unsigned char cmd[5] = {FLAG, ADDR_ER, DISC, BCC(ADDR_ER, DISC), FLAG};
                if (sendAndWaitMessage(fd, cmd, 5) < 0)
                {
                    return -1;
                }
            }
            return 0;
        }
    }

    alarm(0);
    return -1;
}

