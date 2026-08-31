#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdbool.h>

#define MAX_FILE_SIZE 1000000 

bool is_txt_extension(const char *filepath);
long read_file_content(const char *filepath, char *buffer);
int save_received_file(const char *filename, const char *content, long size);

#endif
