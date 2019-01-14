#include "fnv.h"
#include <stdio.h>
#include <string.h>

// hash function
unsigned int FNV32(const char *s, int hash_prime) {
    unsigned int hash = FNV_OFFSET_32;
    int i;

    for (i = 0; i < strlen(s); i++) {
        hash = hash ^ (s[i]); /* xor next byte into the bottom of the hash*/

        /* Multiply by prime number found to work well*/
        hash = hash * FNV_PRIME_32;
    }

    return hash % hash_prime;
}