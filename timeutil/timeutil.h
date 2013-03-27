/*
 * +-----------------------------------------------------------------------+
 * | Defines some utilities for struct timeval operations.                 |
 * +-----------------------------------------------------------------------+
 * | Author: nosqldev@gmail.com                                            |
 * +-----------------------------------------------------------------------+
 */

#ifndef _TIMEUTIL_H_
#define _TIMEUTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif 

/*
 * INCLUDE FILES
 */
#include <time.h>
#include <sys/time.h>

/*
 * MACROS
 */

/* #################### OPERATIONS ON TIMEVALS #################### */
#ifndef timeval_clear
#define    timeval_clear(tvp)        ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#endif /* ! timeval_clear */

#ifndef timeval_isset
#define    timeval_isset(tvp)        ((tvp)->tv_sec || (tvp)->tv_usec)
#endif /* ! timeval_isset */

#ifndef timeval_cmp
#define timeval_cmp(tvp, uvp, cmp)          \
    (((tvp)->tv_sec == (uvp)->tv_sec) ?     \
        ((tvp)->tv_sec cmp (uvp)->tv_sec):  \
        ((tvp)->tv_usec cmp (uvp)->tv_usec))
#endif /* ! timeval_cmp */

#ifndef timeval_add
#define timeval_add(vvp, uvp)               \
    do {                                    \
        (vvp)->tv_sec += (uvp)->tv_sec;     \
        (vvp)->tv_usec += (uvp)->tv_usec;   \
        if ((vvp)->tv_usec >= 1000000){     \
            (vvp)->tv_sec++;                \
            (vvp)->tv_usec -= 1000000;      \
        }                                   \
    } while(0)
#endif /* ! timeval_add */

#ifndef timeval_sub
#define timeval_sub(vpa, vpb, vpc)                          \
    do{                                                     \
        (vpc)->tv_sec = (vpa)->tv_sec - (vpb)->tv_sec;      \
        (vpc)->tv_usec = (vpa)->tv_usec - (vpb)->tv_usec;   \
        if ((vpc)->tv_usec < 0){                            \
            (vpc)->tv_sec --;                               \
            (vpc)->tv_usec += 1000000;                      \
        }                                                   \
    }while (0)
#endif /* ! timeval_sub */

/* #################### OPERATIONS ON TIMESPECS #################### */

#ifndef timespec_clear
#define    timespec_clear(tvp)    ((tvp)->tv_sec = (tvp)->tv_nsec = 0)
#endif /* ! timespec_clear */

#ifndef timespec_isset
#define    timespec_isset(tvp)    ((tvp)->tv_sec || (tvp)->tv_nsec)
#endif /* ! timespec_isset */

#ifndef timespec_cmp
#define    timespec_cmp(tvp, uvp, cmp)                    \
    (((tvp)->tv_sec == (uvp)->tv_sec) ?                \
        ((tvp)->tv_nsec cmp (uvp)->tv_nsec) :            \
        ((tvp)->tv_sec cmp (uvp)->tv_sec))
#endif /* ! timespec_cmp */

#ifndef timespec_add
#define timespec_add(vvp, uvp)                        \
    do {                                              \
        (vvp)->tv_sec += (uvp)->tv_sec;               \
        (vvp)->tv_nsec += (uvp)->tv_nsec;             \
        if ((vvp)->tv_nsec >= 1000000000) {           \
            (vvp)->tv_sec++;                          \
            (vvp)->tv_nsec -= 1000000000;             \
        }                                             \
    } while (0)
#endif /* ! timespec_add */

#ifndef timespec_sub
#define timespec_sub(vvp, uvp)                        \
    do {                                \
        (vvp)->tv_sec -= (uvp)->tv_sec;                \
        (vvp)->tv_nsec -= (uvp)->tv_nsec;            \
        if ((vvp)->tv_nsec < 0) {                \
            (vvp)->tv_sec--;                \
            (vvp)->tv_nsec += 1000000000;            \
        }                            \
    } while (0)
#endif /* ! timespec_sub */

/* #################### TIMER #################### */
#define pre_timer() struct timeval start, end, used
#define launch_timer() gettimeofday(&start, NULL);
#define stop_timer() gettimeofday(&end, NULL);
#define print_timer(PROMPT) do{ \
    timeval_sub(&end, &start, &used);   \
    printf(PROMPT"[used time] %ld.%06ld\n", used.tv_sec, used.tv_usec);   \
}while (0)

/* #################### SLEEP #################### */
#define safe_sleep(s, ms)   do  {   \
    struct timespec t;  \
    t.tv_sec = s;       \
    t.tv_nsec = (ms * 1000000); \
    if (0) { printf("sleep %d.%03d seconds\n", s, ms); } \
    nanosleep(&t, NULL);        \
}   \
while (0)

#ifdef __cplusplus
}
#endif 

#endif /* ! _TIMEUTIL_H_ */
/* vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker: */
