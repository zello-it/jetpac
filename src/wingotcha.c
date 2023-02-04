#ifdef WIN32
#include "wingotcha.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef void* (*pthread_fun)(void*);

typedef struct {
    pthread_fun fun;
    void* args;
}ThreadData;

static DWORD threadMain(void* arg) {
    ThreadData* t = (ThreadData*)arg;
    pthread_fun f = t->fun;
    void* args = t->args;
    free(t);
    return (DWORD)(f(args));
}

int pthread_create(pthread_t * thread,
                          const pthread_attr_t * attr,
                          void *(*start_routine)(void *),
                          void * arg)
{
    assert(attr == NULL);
    *thread = 0;
    ThreadData* td = malloc(sizeof(ThreadData));
    if (td) {
        td->args = arg;
        td->fun = start_routine;
        thread = CreateThread(NULL, 0, threadMain, td, 0, NULL);
    }
    return(!thread);
}
void pthread_exit(void* retval) {
    ExitThread((DWORD)retval);
}
void pthread_join(pthread_t thread, void** retval) {
    WaitForSingleObject(thread, INFINITE);
}

void pthread_mutex_init(pthread_mutex_t* mtx) {
    InitializeCriticalSection(mtx);
}
void pthread_mutex_destroy(pthread_mutex_t* mtx) {
    DeleteCriticalSection(mtx);
}
void pthread_mutex_lock(pthread_mutex_t* mtx) {
    EnterCriticalSection(mtx);
}
void pthread_mutex_unlock(pthread_mutex_t* mtx) {
    LeaveCriticalSection(mtx);
}

void bzero(void* ptr, size_t sz) {
    memset(ptr, 0, sz);
}

LONG atomic_store(LONG* atomic, LONG value) {
    return InterlockedExchange(atomic, value);
}
LONG atomic_load(LONG* atomic) {
    return atomic_store(atomic, *atomic);
}
LONG atomic_fetch_add(LONG* atomic, LONG value) {
    return InterlockedAdd(atomic, value);
}

void usleep(DWORD waitTime) {
    LARGE_INTEGER perfCnt, start, now;

    QueryPerformanceFrequency(&perfCnt);
    QueryPerformanceCounter(&start);

    do {
        QueryPerformanceCounter((LARGE_INTEGER*)&now);
    } while ((now.QuadPart - start.QuadPart) / (float)(perfCnt.QuadPart) * 1000 * 1000 < waitTime);
}
#endif