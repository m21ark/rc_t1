#include "message.h"

extern int (*set_state[])(unsigned char c);
static int (*set_state_fun)(unsigned char c);

int alarm_flag = 0;
static int rcv_paket_nr = 0;

static int nRtr;
static int timeout;

void alarm_handler()
{
    alarm_flag = 1;
}

void set_rcv_packet_nr(int rcv_paket)
{
    rcv_paket_nr = rcv_paket;
}

int sendAndWaitMessage(int fd, unsigned char *msg, int messageSize)
{
    int numTries = 0;
    int ret;

    (void)signal(SIGALRM, alarm_handler); // TODO : NOT HERE

    do
    {
        alarm_flag = 0;

        ret = write(fd, msg, messageSize);
        printf("\nAttempt to send messsage nº%d\n", ++numTries);
        SET_ALARM_TIME(timeout)

        if (ret < messageSize)
        {
            // IMPORTANTE: acaba de escrever caso não tenha sido possível acabar.
            // Tal pode acontecer,px. , devido a limites físicos do buffer da serial port
            // printf("\n LET ME BE FREE TO FINISH THIS PHRASE PLEASE\n");
            int restToWrite = messageSize - ret;

            do
            {
                int rt = write(fd, msg + ret, restToWrite);
                ret += rt;
                if (rt == restToWrite)
                {
                    break;
                }
                restToWrite -= rt;
            } while (!alarm_flag);
        }

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

            DEBUG_PRINT("rt:%d\n", rt);
            set_set_state(set_lookup_transitions(st, rt));

            DEBUG_PRINT("state:%d:%d:%c\n", get_set_state(), buf, buf);

            if (get_set_state() == EXIT_SET_STATE)
            {
                printf("UA/RR/REJ/DISC RECIEVED\n");
                // IF REJ IS RECIEVED WE DONT WANT TO WAIT 3 seconds ... that's why we directly go to the final state
                break;
            }
        }

    } while (numTries < nRtr && get_set_state() != EXIT_SET_STATE);

    TURN_OFF_ALARM

    if (get_set_state() != EXIT_SET_STATE)
    {
        printf("\nFailed to get a response.\n");
        ret = -1;
    }

    DEBUG_PRINT("Setting State to EntryState\n");
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
        DEBUG_PRINT("Everything ok");
        return 0;
    }
    if (ret < 0)
    {
        DEBUG_PRINT("Number of restranmissions was exceeded");
        return -1;
    }

    DEBUG_PRINT(" REJ WAS RECEIVED");
    return 1;
}

int readMessageWithResponse(int fd)
{
    unsigned char buf = 0;
    unsigned char bytes;

    SET_ALARM_TIME(MAX_IDLE_TIME)
    alarm_flag = 0;

    while (!alarm_flag) // SEE THIS LATTER :: É importante uma vez que assim fica a ler lixo quando à barulho
    {
        bytes = read(fd, &buf, 1);
        if (bytes == 0)
        {
            DEBUG_PRINT("Nothing was read\n");
            continue;
        }

        enum set_state_codes st = get_set_state();
        set_state_fun = set_state[st];
        enum set_ret_codes rt = set_state_fun(buf);

        if (rt == BCC2_NOT_OK)
        {
            DEBUG_PRINT("BCC2 Not ok...\n");
        }

        set_set_state(set_lookup_transitions(st, rt));

        if (get_set_state() != 6)
        {
            DEBUG_PRINT("rt:%d | state:%d:%d:%c\n", rt, get_set_state(), buf, buf);
        }
        else
        {
            DEBUG_PRINT(" message>|%c|\n", buf);
        }

        if (get_set_state() == EXIT_SET_STATE)
        {
            DEBUG_PRINT("SET RECIEVED\n");
            set_set_state(ENTRY_SET_STATE);
            TURN_OFF_ALARM
            if (get_control() == SET)
            {
                unsigned char cmd[5] = {FLAG, ADDR_ER, UA, BCC(ADDR_ER, UA), FLAG};
                write(fd, cmd, 5);
                DEBUG_PRINT("Sent a SET");
            }
            else if (get_control() == CTRL_S(0) || get_control() == CTRL_S(1))
            {
                if (get_control() != CTRL_S(rcv_paket_nr))
                { // sends receiver ready when packet is the same as the previous packet recieved
                    unsigned char cmd[5] = {FLAG, ADDR_ER, RR(rcv_paket_nr), BCC(ADDR_ER, RR(rcv_paket_nr)), FLAG};
                    write(fd, cmd, 5);

                    continue;
                }
                return get_data_size();
            }
            else if (get_control() == DISC)
            {
                DEBUG_PRINT("DISC recieved\n");
                unsigned char cmd[5] = {FLAG, ADDR_ER, DISC, BCC(ADDR_ER, DISC), FLAG};
                if (sendAndWaitMessage(fd, cmd, 5) < 0)
                {
                    DEBUG_PRINT("Final UA from Tx wasnt received correctly\n");
                    return -1;
                }
            }
            return 0;
        }
    }

    DEBUG_PRINT("Returning -1\n");
    TURN_OFF_ALARM
    return 0;
}

void set_nr_retransmissions(int nr_retransmissions_r)
{
    nRtr = nr_retransmissions_r;
}
void set_nr_timeout(int timeout_r)
{
    timeout = timeout_r;
}
