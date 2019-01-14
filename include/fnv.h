#ifndef FNV_H
#define FNV_H

#define FNV_PRIME_32 16777619
#define FNV_OFFSET_32 2166136261U

#ifdef __cplusplus
extern "C" {
#endif

// FNV hashing function - input[string] , output[integer]
unsigned int FNV32(const char *s, int hash_prime);

#ifdef __cplusplus
}
#endif

#endif // FNV_H