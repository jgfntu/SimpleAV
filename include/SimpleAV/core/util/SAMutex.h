#ifndef __SIMPLEAV_CORE_UTIL_SAMUTEX_H_DEFINED__
#define __SIMPLEAV_CORE_UTIL_SAMUTEX_H_DEFINED__

/*
 * A simple cross-platform (sorry, *BSD...) mutex.
 *
 * usage:
 * SAMutex mutex;
 * SAMutex_init(&mutex);
 * SAMutex_lock(&mutex);
 * SAMutex_unlock(&mutex);
 * SAMutex_destroy(&mutex);
 *
 * all functions return 0 on success, -1 on error.
*/

// #if defined(__linux__)
// #if defined(_POSIX_VERSION)
#if defined(__unix__)
// #if defined(__USE_POSIX)
// #if defined(_XOPEN_SOURCE)
     #include <pthread.h>
     //typedef pthread_mutex_t SAMutex;
     #define SAMutex pthread_mutex_t
#elif defined(_WIN32)
     #include <windows.h>
     #include <process.h>
     //typedef HANDLE SAMutex;
     #define SAMutex HANDLE
#endif

int SAMutex_init(SAMutex *);
int SAMutex_lock(SAMutex *);
int SAMutex_unlock(SAMutex *);
int SAMutex_destroy(SAMutex *);

#endif