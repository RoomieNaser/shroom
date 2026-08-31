#ifndef CIPHER_H
#define CIPHER_H
#include <stddef.h>

void xor_cipher(char *data, size_t data_len, const char *key);

#endif
