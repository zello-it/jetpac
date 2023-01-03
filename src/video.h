#pragma once
#include "data.h"
#include <pthread.h>
#include <stdatomic.h>

extern pthread_t thread;
extern atomic_bool terminate;

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
Attrib getAttrib(byte col, byte row);
void squareOut(Coords coords, const byte* square, Attrib attrib);

void playSound(byte pitch, byte duration);

word getGameTime();
void resetGameTime();