#include "SimpleAV/core/util/SAMutex.h"

/*
 * A simple cross-platform(sorry, *BSD...) mutex.
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

int SAMutex_init(SAMutex *mutex)
{
     if(mutex == NULL)
          return -1;
#if defined(__unix__)
     return pthread_mutex_init(mutex, NULL) == 0 ? 0 : -1;
#elif defined(_WIN32)
     *mutex = CreateMutex(0, FALSE, 0);
     return *mutex == NULL ? -1 : 0;
#endif
     return -1;
}

int SAMutex_lock(SAMutex *mutex)
{
     if(mutex == NULL)
          return -1;
#if defined(__unix__)
     return pthread_mutex_lock(mutex) == 0 ? 0 : -1;
#elif defined(_WIN32)
     return WaitForSingleObject(*mutex, INFINITE) == WAIT_FAILED ? -1 : 0;
#endif
     return -1;
}

int SAMutex_unlock(SAMutex *mutex)
{
     if(mutex == NULL)
          return -1;
#if defined(__unix__)
     return pthread_mutex_unlock(mutex) == 0 ? 0 : -1;
#elif defined(_WIN32)
     return ReleaseMutex(*mutex) != 0 ? 0 : -1;
#endif
     return -1;
}

int SAMutex_destroy(SAMutex *mutex)
{
     if(mutex == NULL)
          return -1;
#if defined(__unix__)
     return pthread_mutex_destroy(mutex) == 0 ? 0 : -1;
#elif defined(_WIN32)
     return CloseHandle(mutex) != 0 ? 0 : -1;
#endif
     return -1;
}
