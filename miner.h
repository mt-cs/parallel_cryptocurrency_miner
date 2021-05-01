/**
 * @file mine.h
 */

#ifndef _MINER_H_
#define _MINER_H_
#include <stdint.h>

/**
 * Retrieves the current wall clock time, in seconds.
 *
 * @return current wall clock time
 */
double get_time();

/**
 * Prints a 32-bit unsigned integer as binary digits.
 *
 * @param num The 32-bit unsigned integer to print
 */
void print_binary32(uint32_t num);

/**
 * Converting user input to int
 * @param num user input char*
 */
int get_strtol(char *num);

int get_bitcount(unsigned char x);

#endif
