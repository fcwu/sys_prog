#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
main()
{
     struct timeval tv;
     struct timezone tz;
     struct tm *tm;
     gettimeofday(&tv, &tz);
     tm = localtime(&tv.tv_sec);
     printf("%d:%02d:%02d %d \n", tm->tm_hour, tm->tm_min,
              tm->tm_sec, tv.tv_usec);
     printf("%ld.%ld\n", tv.tv_sec, tv.tv_usec);
     exit(0);
}
