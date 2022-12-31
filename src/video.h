#pragma once
#include "data.h"
#include <pthread.h>

extern pthread_t thread;
extern pthread_mutex_t termination_mutex;

typedef union{
    byte attrib;
    struct {
        byte ink:3;
        byte paper:3;
        byte bright:1;
        byte flash:1;
    };
} Attrib;

void initScreen(void);
void renderLoop(void);
void checkTermination(void);
void terminateScreen(void);

void clearScreen(Attrib attrib);
void textOut(Coords coords, const char* msg, Attrib attrib); 
void setAttrib(byte col, byte row, Attrib attrib);