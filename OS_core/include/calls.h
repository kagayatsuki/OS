#ifndef OS_VFS_DEBUG_CALLS_H
#define OS_VFS_DEBUG_CALLS_H

#define winPlatform

#ifdef winPlatform  //platform chosen
#include <windows.h>

typedef HANDLE _core_mutex;
#define _core_mutex_create(mutex) (mutex = CreateMutex(nullptr, false, NULL), \
                                    GetLastError() == ERROR_ALREADY_EXISTS)
#define _core_mutex_lock(mutex) WaitForSingleObject(mutex, INFINITE)
#define _core_mutex_trylock(mutex) WaitForSingleObject(mutex, 50)
#define _core_mutex_unlock(mutex) ReleaseMutex(mutex)
#define _core_mutex_destroy(mutex) CloseHandle(mutex)
#else   //linux
#include <pthread.h>

typedef pthread_mutex_t _core_mutex;
#define _core_mutex_create(mutex) (pthread_mutex_init(&mutex, NULL), mutex_addr)
#define _core_mutex_lock(mutex) pthread_mutex_lock(&mutex)
#define _core_mutex_trylock(mutex) pthread_mutex_trylock(&mutex)
#define _core_mutex_unlock(mutex) pthread_mutex_unlock(&mutex)
#define _core_mutex_destroy(mutex) pthread_mutex_destroy(&mutex)
#endif

#endif //OS_VFS_DEBUG_CALLS_H
