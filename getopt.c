#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
    int ch;
    int optidx;
    static int daggerset = 0, daggerset2 = 1;

    /* options descriptor */
    static struct option longopts[] = {
        { "b1",   no_argument,            NULL,           'b' },
        { "f1",   required_argument,      NULL,           'f' },
        { "f2",   required_argument,      NULL,           'f' },
        { "d1",   no_argument,            &daggerset,     1 },
        { "d2",   no_argument,            &daggerset2,    0 },
        { NULL,   0,                      NULL,           0 }
    };

    while ((ch = getopt_long(argc, argv, "bf:p::::t:", longopts, &optidx)) != -1) {
        printf("Opt #%d for 0x'%02X, %d: ", optind, ch, optidx);
        switch (ch) {
            case 'b':
                printf("-b\n");
                break;
            case 'f':
                printf("%s %s\n", argv[optind - 2], optarg);
                break;
            case 'p':
                printf("%s %s %s %s %s\n", argv[optind - 1], argv[optind], argv[optind + 1], argv[optind + 2], argv[optind + 3]);
                break;
            case 't':
                printf("%s\n", argv[optind - 1]);
            case 0:
                printf("\n");
                break;
            default:
                printf("Usage\n");
                break;
        }
    }
    printf("d1: %d, d2: %d\n", daggerset, daggerset2);
    argc -= optind;
    argv += optind;
    printf("argc: %d, argv: %s\n", argc, *argv);

    return 0;
}
