#pragma once
#include "data.h"
#ifndef WIN32
#include <pthread.h>
#include <stdatomic.h>
#else
#include "wingotcha.h"
#endif

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
void textOutAttrib(Coords coords, const char* msg, Attrib attrib);
void textOut(Coords coords, const char* msg); 
void setAttrib(byte col, byte row, Attrib attrib);
Attrib getAttrib(byte col, byte row);
byte getVideoByte(Coords coords);
void squareOut(Coords coords, const byte* square, Attrib attrib);
void lockVideo();
void unlockVideo();


enum Operator{
    EQUAL,
    OR,
    XOR,
    AND
};
void byteOut(Coords coords, const byte byte, enum Operator op);
void byteOutNoLock(Coords coords, const byte byte, enum Operator op);

void playSound(byte pitch, byte duration);

word getGameTime(void);
void resetGameTime(void);
void gameSleep(int millis);
int getMillis(void);
void ei(void);
void di(void);
