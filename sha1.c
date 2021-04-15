#include <stdio.h>

#include "sha1.h"

/******************** See RFC 4634 for details ******************/
/*
*  Description:
*      This file implements the Secure Hash Signature Standard
*      algorithms as defined in the National Institute of Standards
*      and Technology Federal Information Processing Standards
*      Publication (FIPS PUB) 180-1 published on April 17, 1995, 180-2
*      published on August 1, 2002, and the FIPS PUB 180-2 Change
*      Notice published on February 28, 2004.
*
*      A combined document showing all algorithms is available at
*              http://csrc.nist.gov/publications/fips/
*              fips180-2/fips180-2withchangenotice.pdf
*
*      The SHA-1 algorithm produces a 160-bit message digest for a
*      given data stream.  It should take about 2**n steps to find a
*      message with the same digest as a given message and
*      2**(n/2) to find any two messages with the same digest,
*      when n is the digest size in bits.  Therefore, this
*      algorithm can serve as a means of providing a
*      "fingerprint" for a message.
*
*  Portability Issues:
*      SHA-1 is defined in terms of 32-bit "words".  This code
*      uses <stdint.h> (included via "sha.h") to define 32 and 8
*      bit unsigned integer types.  If your C compiler does not
*      support 32 bit unsigned integers, this code is not
*      appropriate.
*
*  Caveats:
*      SHA-1 is designed to work with messages less than 2^64 bits
*      long. This implementation uses SHA1Input() to hash the bits
*      that are a multiple of the size of an 8-bit character, and then
*      uses SHA1FinalBits() to hash the final few bits of the input.
*/

typedef struct SHA1Context {
    uint32_t Intermediate_Hash[SHA1_HASH_SIZE/4];
    uint32_t Length_Low;
    uint32_t Length_High;
    int_least16_t Message_Block_Index;
    uint8_t Message_Block[64];
    int Computed;
    int Corrupted;
} SHA1Context;

int SHA1Reset(SHA1Context *);
int SHA1Input(SHA1Context *, const uint8_t *, unsigned int);
int SHA1Result(SHA1Context *, uint8_t Message_Digest[SHA1_HASH_SIZE]);
void SHA1PadMessage(SHA1Context *);
void SHA1ProcessMessageBlock(SHA1Context *);

#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

int SHA1Reset(SHA1Context *context)
{
    if (!context) {
        return shaNull;
    }
    context->Length_Low           = 0;
    context->Length_High          = 0;
    context->Message_Block_Index  = 0;
    context->Intermediate_Hash[0] = 0x67452301;
    context->Intermediate_Hash[1] = 0xEFCDAB89;
    context->Intermediate_Hash[2] = 0x98BADCFE;
    context->Intermediate_Hash[3] = 0x10325476;
    context->Intermediate_Hash[4] = 0xC3D2E1F0;
    context->Computed   = 0;
    context->Corrupted  = 0;
    return shaSuccess;
}

int SHA1Result(SHA1Context *context, uint8_t Message_Digest[SHA1_HASH_SIZE])
{
    int i;
    if (!context || !Message_Digest) {
        return shaNull;
    }
    if (context->Corrupted) {
        return context->Corrupted;
    }
    if (!context->Computed) {
        SHA1PadMessage(context);
        for(i=0; i<64; ++i) {
            /* message may be sensitive, clear it out */
            context->Message_Block[i] = 0;
        }
        context->Length_Low = 0;    /* and clear length */
        context->Length_High = 0;
        context->Computed = 1;
    }
    for(i = 0; i < SHA1_HASH_SIZE; ++i) {
        Message_Digest[i]
            = context->Intermediate_Hash[i>>2] >> 8 * ( 3 - ( i & 0x03 ) );
    }
    return shaSuccess;
}

int SHA1Input(SHA1Context *context,
        const uint8_t *message_array, unsigned length)
{
    if (!length) {
        return shaSuccess;
    }
    if (!context || !message_array) {
        return shaNull;
    }
    if (context->Computed) {
        context->Corrupted = shaStateError;
        return shaStateError;
    }
    if (context->Corrupted) {
         return context->Corrupted;
    }
    while(length-- && !context->Corrupted) {
        context->Message_Block[context->Message_Block_Index++]
            = (*message_array & 0xFF);
        context->Length_Low += 8;
        if (context->Length_Low == 0) {
            context->Length_High++;
            if (context->Length_High == 0) {
                /* Message is too long */
                context->Corrupted = 1;
            }
        }
        if (context->Message_Block_Index == 64) {
            SHA1ProcessMessageBlock(context);
        }
        message_array++;
    }
    return shaSuccess;
}

void SHA1ProcessMessageBlock(SHA1Context *context)
{
    const uint32_t K[] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
    int t;
    uint32_t temp;
    uint32_t W[80];
    uint32_t A, B, C, D, E;
    for(t = 0; t < 16; t++) {
        W[t] = context->Message_Block[t * 4] << 24;
        W[t] |= context->Message_Block[t * 4 + 1] << 16;
        W[t] |= context->Message_Block[t * 4 + 2] << 8;
        W[t] |= context->Message_Block[t * 4 + 3];
    }
    for(t = 16; t < 80; t++) {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }
    A = context->Intermediate_Hash[0];
    B = context->Intermediate_Hash[1];
    C = context->Intermediate_Hash[2];
    D = context->Intermediate_Hash[3];
    E = context->Intermediate_Hash[4];
    for(t = 0; t < 20; t++) {
        temp = SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    for(t = 20; t < 40; t++) {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    for(t = 40; t < 60; t++) {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    for(t = 60; t < 80; t++) {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;
    context->Message_Block_Index = 0;
}

void SHA1PadMessage(SHA1Context *context)
{
    if (context->Message_Block_Index > 55) {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 64) {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
        SHA1ProcessMessageBlock(context);
        while(context->Message_Block_Index < 56) {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    } else {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 56) {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }
    context->Message_Block[56] = context->Length_High >> 24;
    context->Message_Block[57] = context->Length_High >> 16;
    context->Message_Block[58] = context->Length_High >> 8;
    context->Message_Block[59] = context->Length_High;
    context->Message_Block[60] = context->Length_Low >> 24;
    context->Message_Block[61] = context->Length_Low >> 16;
    context->Message_Block[62] = context->Length_Low >> 8;
    context->Message_Block[63] = context->Length_Low;
    SHA1ProcessMessageBlock(context);
}

int sha1sum(uint8_t digest[SHA1_HASH_SIZE],
        const uint8_t *data, unsigned int length)
{
    int result;

    SHA1Context sha;
    result = SHA1Reset(&sha);
    if (result != shaSuccess) {
        return result;
    }

    result = SHA1Input(&sha, data, length);
    if (result != shaSuccess) {
        return result;
    }

    result = SHA1Result(&sha, digest);
    return result;
}

void sha1tostring(char hash_str[41],
        uint8_t digest[SHA1_HASH_SIZE])
{
    for(int i = 0; i < SHA1_HASH_SIZE ; ++i) {
        hash_str += sprintf(hash_str, "%02X", digest[i]);
    }
}
