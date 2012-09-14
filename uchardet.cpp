/**
 * build: g++ uchardet.cpp `pkg-config --libs uchardet` -o uchardet
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchardet/uchardet.h>

const char * get_file_encoding(const char* in_str, unsigned int str_len){    
    const char * enc_name;
    uchardet_t handler = uchardet_new();
    if (uchardet_handle_data(handler, in_str, str_len) == 0) {
        uchardet_data_end(handler);
        enc_name = uchardet_get_charset(handler);
        if (enc_name[0] == 0) {
            enc_name = NULL;
        }
    } else {
        enc_name = NULL;
    }
    uchardet_delete(handler);
    return enc_name;
}    

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        exit(0);
    }
    FILE * fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("file not found\n");
        exit(1);
    }
    char str[1024];
    int len = 0;
    len = fread(str, sizeof(char), sizeof(str) / sizeof(char), fp);
    fclose(fp);
    if (len <= 0) {
        printf("No data in file\n");
        exit(2);
    }
    const char * enc_name = get_file_encoding(str, len);
    if (enc_name != NULL) {
        printf("detect charset is [%s]\n", enc_name);
    } else {
        printf("Failed to detect charset\n");
    }

    return 0;
}
