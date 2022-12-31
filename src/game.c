#include "game.h"
#include "data.h"
#include "video.h"
#include <stdbool.h>
#include <strings.h>

#define ZEROSTRUCT(name, type) (name = (const type){0})
#define ZEROARRAY(name, type) {for(int n=0; n < array_sizeof(name); ++n){ZEROSTRUCT(name[n], type);}}
void resetGlobals() {
    // zero all but high scores
    ZEROSTRUCT(gameOptions, GameOptions);
    bzero(p1Score, sizeof(p1Score));
    bzero(p2Score, sizeof(p2Score));
    ZEROSTRUCT(jetmanState, ActorState);
    ZEROARRAY(laserBeamParam, LaserBeam);
    ZEROSTRUCT(explosionSfxParams, SoundData);
    ZEROSTRUCT(rocketModuleState, ModuleState);
    ZEROSTRUCT(itemState, ModuleState);
    ZEROSTRUCT(jetmanThrusterAnimState, AnimState);
    ZEROARRAY(alienState, ActorState);
    ZEROSTRUCT(jetmanExplodingAnimState, AnimState);
    ZEROSTRUCT(inactiveJetmanState, ActorState);
    ZEROARRAY(inactiveRocketState, ActorState);
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

// check termination

// menuScreen
void menuScreen() {
    checkTermination();

}

void gameLoop(void) {
    checkTermination();
}

void newGame(void) {
    gameLoop();
}
void resetScreen(void){
    clearScreen((Attrib){.ink = 7, .paper = 0, .bright = 1}); 
    textOut((Coords){.x = 24, .y = 0}, "1UP", (Attrib){.attrib = 0x47});
    textOut((Coords){.x = 120,.y = 0}, "HI", (Attrib){.attrib = 0x45});
    textOut((Coords){.x = 216, .y = 0}, "2UP", (Attrib){.attrib = 0x47});
    for(byte row = 0; row < 32; ++ row) {
        setAttrib(1, row, (Attrib){.attrib = 0x46});
    }
}

void startGame(void) {
    resetGlobals();
    resetScreen();
    jetmanSpeedModifier = 0x4;
    menuScreen();
    newGame();
}