#ifndef _RCOM_UTILS_H
#define _RCOM_UTILS_H

#include <stdio.h>
#include "macros.h"

/**
 * @brief Counts how many flags in given data need an escape byte before them
 *
 * @param data buffer to be analyzed
 * @param dataSize size of buffer given
 * @return int number of escape bytes needed
 */
int countProblematicFlags(unsigned char *data, int dataSize);

/**
 * @brief Stuffs given data by adding escape bytes where needed
 *
 * @param data buffer to be stuffed
 * @param dataSize size of buffer
 * @param stData output buffer stuffed
 * @param stSize size of buffer
 * @return int returns 0 on success
 */
int stuffData(unsigned char *data, int dataSize, unsigned char *stData, int stSize);

/**
 * @brief Removes escape bytes on problematic flags
 *
 * @param data output buffer unstuffed
 * @param dataSize size of unstuffed data
 * @param stData stuffed buffer
 * @return int
 */
int unstuffData(unsigned char *data, int dataSize, unsigned char *stData);

/**
 * @brief Verifies of error occured by applying XOR operations on given data
 *
 * @param data buffer to be checked
 * @param dataSize size of buffer
 * @return unsigned char result of buffer analyze
 */
unsigned char BCC2(const unsigned char *data, int dataSize);

#endif // _RCOM_UTILS_H