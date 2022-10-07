#include "../include/set_st.h"

unsigned char msg[5] = {0};

int (*set_state[])(unsigned char c) = {
    set_entry_state, set_flag_state, set_a_state, set_c_state, set_bcc_state, set_stop_state, set_data_state};

int set_entry_state(unsigned char c)
{
    enum set_ret_codes ret = OTHER_RCV;
    switch (c)
    {
    case FLAG:
        ret = FLAG_RCV;
        msg[0] = c;
        break;
    default:
        break;
    }
    return ret;
}

int set_flag_state(unsigned char c)
{
    enum set_ret_codes ret = OTHER_RCV;
    switch (c)
    {
    case ADDR_ER: // A UA VAI SER DIFERENTE
        ret = A_RCV;
        msg[1] = c;
        break;
    case FLAG:
        ret = FLAG_RCV;
        break;
    default:
        break;
    }
    return ret;
}

int set_a_state(unsigned char c)
{
    // ----------------------------------------------------------------------
    // TODO ::: AQUI VAI TER UM ERRO ... basicamente só posso dizer que o tx e rx esta ready
    // quando for confirmado de que n houve erros na trama MUDARRRRRRR
    // ----------------------------------------------------------------------

    enum set_ret_codes ret = OTHER_RCV;
    static int tx_ready_to_send = 0;
    static int rx_RR = 0;
    switch (c)
    {
    case SET: // A UA VAI SER DIFERENTE
        rx_RR = 1;
        ret = C_RCV;
        msg[2] = c;
        break;
    case UA:
        tx_ready_to_send = 1; // TX can recieve RR
        ret = C_RCV;
        msg[2] = c;
        break;
    case RR(0):
    case RR(1):
        if (tx_ready_to_send) // just in case tx has not yet made pass the "handshake" fase
        {
            ret = C_RCV;
            msg[2] = c;
        }
        break;
    case CTRL_S(1):
    case CTRL_S(0):
        if (rx_RR)
        {
            ret = C_RCV;
            msg[2] = c;
        }
        break;
    case FLAG:
        ret = FLAG_RCV;
        break;
    default:
        break;
    }
    return ret;
}

int set_c_state(unsigned char c)
{
    enum set_ret_codes ret = OTHER_RCV;
    switch (c)
    {
    case FLAG:
        ret = FLAG_RCV;
        break;
    default:
        break;
    }

    unsigned char validation = BCC(msg[1], msg[2]);
    if (memcmp(&c, &validation, 1) == 0)
    {
        ret = BCC_OK;
        msg[3] = c;

        if (msg[2] == CTRL_S(1) || msg[2] == CTRL_S(0))
        {
            ret = INF_FRAME;
            data_size = 0;
        }
    }

    return ret;
}

int set_bcc_state(unsigned char c)
{
    enum set_ret_codes ret = OTHER_RCV;
    switch (c)
    {
    case FLAG:
        ret = FLAG_RCV;
        msg[4] = c;
        break;
    default:
        break;
    }
    return ret;
}

int set_stop_state(unsigned char c)
{
    return OTHER_RCV;
}

int set_data_state(unsigned char c)
{
    enum set_ret_codes ret = OTHER_RCV;
    switch (c)
    {
    case FLAG:
        ret = FLAG_RCV;
        msg[3] = msg[4]; // BCC2
        break;
    default:
        break;
    }

    msg[4] = c;

    if (ret == OTHER_RCV)
    {
        sdata[data_size++] = c;
    }

    // save message data

    return ret;
}

enum set_state_codes set_lookup_transitions(int cur_state, int rc)
{
    SET_ST_TRANS state_transitions[] = {
        {start, FLAG_RCV, flag_rcv},
        {start, OTHER_RCV, start},
        {flag_rcv, FLAG_RCV, flag_rcv},
        {flag_rcv, A_RCV, a_rcv},
        {flag_rcv, OTHER_RCV, start},
        {a_rcv, FLAG_RCV, flag_rcv},
        {a_rcv, OTHER_RCV, start},
        {a_rcv, C_RCV, c_rcv},
        {c_rcv, FLAG_RCV, flag_rcv},
        {c_rcv, OTHER_RCV, start},
        {c_rcv, BCC_OK, bcc_ok},
        {bcc_ok, FLAG_RCV, stop},
        {bcc_ok, OTHER_RCV, start},
        {stop, OTHER_RCV, stop},
        {c_rcv, INF_FRAME, data},
        {data, OTHER_RCV, data},
        {data, FLAG_RCV, stop}};

    for (int i = 0; i < 17; i++)
    {
        if (state_transitions[i].src_state == cur_state && state_transitions[i].ret_code == rc)
        {
            return state_transitions[i].dst_state;
        }
    }
    return cur_state;
}

enum set_state_codes get_set_state()
{
    return set_cur_state;
}

void set_set_state(enum set_state_codes st)
{
    set_cur_state = st;
}

unsigned char get_control()
{
    return msg[2];
}

int get_data_size()
{
    return data_size;
}
void get_data(unsigned char *dt)
{
    memcpy(sdata, dt, data_size);
}
