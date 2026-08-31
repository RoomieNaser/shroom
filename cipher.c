#include "cipher.h"
#include <string.h>

void xor_cipher(char *data, size_t data_len, const char *key) {
	size_t key_len = strlen(key);
	if (key_len == 0 || data_len == 0) return;

	for (size_t i = 0; i < data_len; i++) {
		data[i] ^= key[i % key_len];
    	}
}
