#include "data.h"
#include "game.h"
#include "video.h"

void* threadMain(void* ignored) {
    startGame();
    return NULL;
}

int main(int argn, char** args) {
    int ret = 0;
    initScreen(); 
    if(!pthread_create(&thread, NULL, threadMain, NULL))
    {
        renderLoop();
    }
    terminateScreen();
    return ret;
}