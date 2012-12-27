#include <stdio.h>
#include <string.h>
#include <iconv.h>


int main() {
    char test_str[] = {
        0x2d, 0x4e, 0x96, 0x89, 0xb0, 0x65, 0x5e, 0x80, 0xf0, 0x53
    };
    //char * test_str = "\x2d\x4e";
    int test_str_len = 10;

    static char buf[128];
    static iconv_t text_conv = (iconv_t)-1;
    char *ip = test_str;
    char *op = buf;
    size_t len1 = test_str_len;
    size_t len2 = sizeof(buf) / sizeof(char);
    
    if (text_conv == (iconv_t)-1) {
        text_conv = iconv_open("UTF-8", "ISO-10646/UCS2");
        if (text_conv == (iconv_t)-1) {
            fprintf(stderr, "error open iconv\n");
            return -1;
        }
    }
    memset(buf, 0, len2--);
    iconv(text_conv, &ip, &len1, &op, &len2);
    printf("len1: %ld, len2: %ld\n", len1, len2);
    printf("%s\n", buf);
    iconv_close(text_conv);
    return 0;
}
