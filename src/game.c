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
void resetScreen(void);
void resetGlobals(void) {
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

byte* writeThreeBytes(byte* what, byte* to, byte shift) {
    byte b1, b2, b3;
    b1 = *what++;
    b2 = *what++;
    b3 = 0;
    while(shift--) {
        byte rep = b1 & 1;
        b1 >>= 1;
        byte rep2 = b2 & 1;
        b2 = (b2 >> 1) + rep;
        b3 = (b3 >> 1) + rep2;
    }
    *to++ = b1;
    *to++ = b2;
    *to++ = b3;
    return to;
}

void bufferCopy(byte* what, byte* where, byte howmanywords, byte shift) {
    while(howmanywords--) {
        where = writeThreeBytes(what, where, shift);
    }
}

void bufferCopyRocket(void) {
    // copia quattro volte lo stesso modulo?
    Sprite* sprite = collectibleSpriteTable[4 + playerLevel / 4];
    Buffer* dest = bufferItem;
    for(byte b = 0; b < 4; ++b) {
        dest->header = 0;
        dest->width = 3;
        dest->height = (sprite->height > 0x11 ? sprite->height : 0x10);
        bufferCopy(sprite->data, dest->pixel, dest->height, 0);
    }
}

void displayPlayerLives(byte numplayer) {
    int lives = (currentPlayerNumber == numplayer ? playerLives : inactivePlayerLives);
    byte pos = (numplayer ? 0xb0 : 0x40);
    Attrib a = {.bright = 1, .ink = 7};
    if(lives) {
        char* buf = numToChar(lives);
        textOut((Coords){.x = pos, .y = 0}, buf, a);
        squareOut((Coords){.x = pos + 8, .y = 0}, tileLifeIcon, a);
    } else {
        textOut((Coords){.x = pos, .y = 0}, "  ", a);
    }
}

void playerInit() {
    jetmanState = defaultPlayerState;
    playerDelayCounter = (gameOptions.players ? 0xff : 0x80);
    --playerLives;
    displayPlayerLives(0);
    displayPlayerLives(1);
}

void drawPlatforms() {
    for(byte b = 0; b < array_sizeof(gfxParamsPlatforms); ++b) {
        GFXParams* platform = &gfxParamsPlatforms[b];
        if(platform->color) {
            Coords coords;
            Attrib attrib = {.attrib = platform->color};
            byte width = platform->width;
            coords.x = platform->x - (width & 0xfc) + 0x10;
            coords.y = platform->y;
            squareOut(coords, tilePlatformLeft, attrib);
            byte counter = platform->width / 4 - 4;
            while(counter-- > 0) {
                coords.x += 8;
                squareOut(coords, tilePlatformMiddle, attrib);
            }
            coords.x += 8;
            squareOut(coords, tilePlatformRight, attrib);
        }
    }
}
void alienBufferInit() {
    rocketModAttached = 4;
    jetmanRocketModConnected = 0;
    byte index = (playerLevel * 2) % 16;
    
}

void levelInit() {
    alienBufferInit();
    resetScreen();
    drawPlatforms();
    displayPlayerLives(0);
    displayPlayerLives(1);
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
    levelInit();
    playerInit();
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