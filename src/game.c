#include "game.h"
#include "data.h"
#include "video.h"
#include "menu.h"
#include <stdbool.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#define ZEROSTRUCT(name, type) (name = (const type){0})
#define ZEROARRAY(name, type) {for(int n=0; n < array_sizeof(name); ++n){ZEROSTRUCT(name[n], type);}}
void resetGlobals() {
    // zero all but high scores and controls
    //ZEROSTRUCT(gameOptions, GameOptions);
    p1Score = p2Score = 0;
    ZEROSTRUCT(jetmanState, State);
    ZEROARRAY(laserBeamParam, LaserBeam);
    ZEROSTRUCT(explosionSfxParams, SoundData);
    ZEROSTRUCT(rocketState, State);
    ZEROSTRUCT(rocketModuleState, State);
    ZEROSTRUCT(itemState, State);
    ZEROSTRUCT(jetmanThrusterAnimState, State);
    ZEROARRAY(alienState, State);
    ZEROSTRUCT(jetmanExplodingAnimState, State);
    ZEROSTRUCT(inactiveJetmanState, State);
    ZEROARRAY(inactiveRocketState, State);
    ZEROSTRUCT(actor, ActorTempState);
    alienNewDirFlag = currentAlienNumber = 0;
    gameTime = 0;
    ZEROSTRUCT(actorCoords, Coords);
    currentPlayerNumber = Player1;
    jetmanRocketModConnected = rocketModAttached = lastFrame = frameTicked = 0;
    playerDelayCounter = 0;
    playerLevel = 0;
    playerLives = inactivePlayerLevel = inactivePlayerLives = 0;
    ZEROARRAY(bufferAliensRight, Buffer);
    ZEROARRAY(bufferAliensLeft, Buffer);
    ZEROARRAY(bufferItem, Buffer);
}

void bufferCopyRocket(void) {
    
}

void gameLoop(void) {
    checkTermination();
}

void newGame(void) {
    playerLevel = 0;
    playerLives = 4;
    rocketState = defaultRocketState[0];
    rocketModuleState = defaultRocketState[1];
    itemState = defaultRocketState[2];
    bufferCopyRocket();
    inactivePlayerLevel = 0;
    if(gameOptions.players) {
        inactivePlayerLives = 5;
    } else {
        inactivePlayerLives = 0;
    }
    gameLoop();
}

void showScore(int line, int col, uint32_t score) {
    char* tmp = NULL;
    size_t sz = 0;
    score &= 0xffffff; //only three bytes
    sz = snprintf(tmp, sz, "%d", score) + 1;
    tmp = malloc(sz);
    snprintf(tmp, sz, "%d", score);
    textOut((Coords){.x = col * 8, .y = line}, tmp, (Attrib){.ink = 7, .bright = 1});
    free(tmp);
}

void resetScreen(void){
    clearScreen((Attrib){.ink = 7, .paper = 0, .bright = 1}); 
    textOut((Coords){.x = 24, .y = 0}, "1UP", (Attrib){.attrib = 0x47});
    textOut((Coords){.x = 120,.y = 0}, "HI", (Attrib){.attrib = 0x45});
    textOut((Coords){.x = 216, .y = 0}, "2UP", (Attrib){.attrib = 0x47});
    for(byte row = 0; row < 32; ++ row) {
        setAttrib(1, row, (Attrib){.attrib = 0x46});
    }
    showScore(8, 1, p1Score);
    showScore(8, 25, p2Score);
    showScore(8, 13, hiScore);
}

void startGame(void) {
    resetGlobals();
    resetScreen();
    jetmanSpeedModifier = 0x4;
    menuScreen();
    newGame();
}