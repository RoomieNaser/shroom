#include "file_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool is_txt_extension(const char *filepath) {
    const char *dot = strrchr(filepath, '.');
    if (!dot || dot == filepath) return false;
    return strcmp(dot, ".txt") == 0;
}

long read_file_content(const char *filepath, char *buffer) {
    FILE *file = fopen(filepath, "r");
    if (!file) return -1;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size > MAX_FILE_SIZE) {
        fclose(file);
        return -2;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);
    return size;
}

int save_received_file(const char *filename, const char *content, long size) {
    char prefixed_name[256];
    snprintf(prefixed_name, sizeof(prefixed_name), "received_%s", filename);

    FILE *file = fopen(prefixed_name, "w");
    if (!file) return -1;

    fwrite(content, 1, size, file);
    fclose(file);
    return 0;
}
