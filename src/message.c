#include "../include/message.h"

extern int (*set_state[])(unsigned char c);

int alarm_flag = 0;

void alarm_handler() {
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
        printf("bytes written\n");
        alarm(3);


        unsigned char buf = 0;
        unsigned char bytes;

        while (!alarm_flag) 
        {
            bytes = read(fd, &buf, 1);
            printf("%d", buf);
            if (bytes == 0) continue;

            enum set_state_codes st = get_set_state();
            set_state_fun = set_state[st];
            enum set_ret_codes rt = set_state_fun(buf);

            printf("rt:%d\n", rt);
            set_set_state(set_lookup_transitions(st, rt));

            printf("state:%d:%d\n", get_set_state(), buf);

            if (get_set_state() == EXIT_SET_STATE)
            {
                printf("UA RECIEVED");
                break;
            }
        }

    } while (numTries < 3 && get_set_state() != EXIT_SET_STATE);


    alarm(0);

    if (get_set_state() != EXIT_SET_STATE) {
        printf("FAILED TO GET RESPONSE!");
    }

    
}
