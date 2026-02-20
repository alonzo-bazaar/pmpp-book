#include<stddef.h>   // NULL
#include<sys/time.h> // gettimeofday
long long current_millis();

#ifdef TIMING_INCLUDE_IMPLEMENTATION
long long current_millis(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}
#endif
