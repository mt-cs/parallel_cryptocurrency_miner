/**
 * @file sha1.h publicly-accessible SHA-1 functions
 */

#ifndef _SHA1_H_
#define _SHA1_H_

#include <stdint.h>

#define SHA1_HASH_SIZE 20

/**
 * Defines various SHA-1 status codes.
 */
enum {
    shaSuccess = 0,
    shaNull,
    shaInputTooLong,
    shaStateError
};

/**
 * Calculates a SHA-1 checksum for the given data.
 *
 * @param digest Array to store the resulting checksum in
 * @param data Data to checksum
 * @param length Data size (in bytes)
 *
 * @return shaSuccess (0) on success, nonzero otherwise
 */
int sha1sum(uint8_t digest[SHA1_HASH_SIZE],
        const uint8_t *data,
        unsigned int length);

/**
 * Converts a SHA-1 checksum in raw byte form to a hexidecimal string.
 *
 * @param hash_str Destination character array to store the hex string. Will be
 *        null terminated.
 * @param digest The SHA-1 checksum to convert (in raw byte form)
 */
void sha1tostring(char hash_str[41],
        uint8_t digest[SHA1_HASH_SIZE]);

#endif
