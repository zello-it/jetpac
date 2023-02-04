#pragma once

#define WIN32_LEAN_AND_MEAN
#if defined(_WIN32)           
#define NOGDI             // All GDI defines and routines
#define NOUSER            // All USER defines and routines
#endif

#include <Windows.h> // or any library that uses Windows.h

#if defined(_WIN32)           // raylib uses these names as function parameters
#undef near
#undef far
#endif


typedef HANDLE pthread_t;
typedef int pthread_attr_t;


int pthread_create(pthread_t * thread,
                          const pthread_attr_t * attr,
                          void *(*start_routine)(void *),
                          void * arg);
void pthread_exit(void* retval);
void pthread_join(pthread_t thread, void** retval);

typedef CRITICAL_SECTION pthread_mutex_t;
#define PTHREAD_MUTEX_INITIALIZER {0}

void pthread_mutex_lock(pthread_mutex_t* mutex);
void pthread_mutex_unlock(pthread_mutex_t* mutex);
void pthread_mutex_init(pthread_mutex_t* mutex);
void pthread_mutex_destroy(pthread_mutex_t* mutex);

typedef LONG atomic_bool;
typedef LONG atomic_uint_least16_t;

LONG atomic_store(LONG* atomic, LONG value);
LONG atomic_load(LONG* atomic);
LONG atomic_fetch_add(LONG* atomic, LONG value);
void bzero(void* mem, size_t sz);
void usleep(int millis);