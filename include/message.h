#ifndef _RCOM_MESSAGE_H_
#define _RCOM_MESSAGE_H_

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "set_st.h"
#include "macros.h"
#include "utils.h"


/**
 * @brief Send a message to the given file descriptor and waits for confirmation response
 * 
 * @param fd where to write message
 * @param msg message to be written
 * @param messageSize size of message to be written
 * @return int TODO
 */
int sendAndWaitMessage(int fd, unsigned char *msg, int messageSize);

/**
 * @brief 
 * 
 * @param fd 
 * @param data 
 * @param dataSize 
 * @param packet 
 * @return int TODO
 */
int sendInformationFrame(int fd, const unsigned char *data, int dataSize, int packet);

/**
 * @brief Reads message from given file descriptor
 * 
 * @param fd file descriptor to be used
 * @return int TODO
 */
int readMessageWithResponse(int fd);

/**
 * @brief Set the reciever packet number object
 * 
 * @param rcv_paket  value to be set
 */
void set_rcv_packet_nr(int rcv_paket);

/**
 * @brief Handler for alarm calling that raises an alarm flag
 * 
 */
void alarm_handler();


void set_nr_retransmissions(int nr_retransmissions);
void set_nr_timeout(int timeout);


#endif // _RCOM_MESSAGE_H_