#include "game.h"
#include "data.h"
#include "video.h"
#include "menu.h"
#include <stdbool.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h> //for keyboard, could be abstracted

#define ZEROSTRUCT(name, type) (name = (const type){0})
#define ZEROARRAY(name, type) {for(int n=0; n < array_sizeof(name); ++n){ZEROSTRUCT(name[n], type);}}

void frameRateLimiter(void);
void gamePlayStarts(void);
void jetmanWalk(void);
void meteorUpdate(void);
void collisionDetection(void);
void crossedShipUpdate(void);
void sphereAlienUpdate(void);
void jetFighterUpdate(void);
void animateExplosion(void);
void rocketUpdate(void);
void rocketTakeoff(void);
void rocketLanding(void);
void sfxEnemyDeath(void);
void sfxJetmanDeath(void);
void itemCheckCollect(void);
void ufoUpdate(void);
void laserBeamAnimate(void);
void squidgyAlienUpdate(void);

typedef void(*Fun)(void);
Fun mainJumpTable[] = {
    frameRateLimiter,
    gamePlayStarts,
    jetmanWalk,
    meteorUpdate,
    collisionDetection,
    crossedShipUpdate,
    sphereAlienUpdate,
    jetFighterUpdate,
    animateExplosion,
    rocketUpdate,
    rocketTakeoff,
    rocketLanding,
    sfxEnemyDeath,
    sfxJetmanDeath,
    itemCheckCollect,
    ufoUpdate,
    laserBeamAnimate,
    squidgyAlienUpdate
};

void newActor(void);
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
    resetGameTime();
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

void bufferCopy(byte* what, Buffer* where, byte height, byte shift) {
    where->header = 0;
    where->width = 3;
    height = (height > 0x11 ? height : 0x10);
    where->height = height;
    byte* ptrto = where->pixel;
    while(height--) {
        ptrto = writeThreeBytes(what, ptrto, shift);
        what += 2;
    }
}

void bufferCopyRocket(void) {
    // copia quattro volte lo stesso modulo?
    Sprite* sprite = collectibleSpriteTable[4 + playerLevel / 4];
    Buffer* dest = bufferItem;
    for(byte b = 0; b < 4; ++b) {
        bufferCopy(sprite->data, dest + b, sprite->height, 0);
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
    playerDelayCounter = (gameOptions.players ? Player2Delay : Player1Delay);
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
    for(byte b = 0; b < 2; ++b) {
        AlienSprite* sprite = alienSpriteTable[index++];
        Buffer* buf = &bufferAliensRight[b];
        bufferCopy(sprite->data, buf, sprite->height, 0);
    }
    for(byte b = 0; b < 2; ++b) {
        AlienSprite* sprite = alienSpriteTable[index++];
        Buffer* buf = &bufferAliensLeft[b];
        bufferCopy(sprite->data, buf, sprite->height, 0);
    }
}

void levelInit(void) {
    currentState = 0;
    alienBufferInit();
    resetScreen();
    drawPlatforms();
    displayPlayerLives(0);
    displayPlayerLives(1);
}

void rocketReset() {
    rocketState = defaultRocketState[0];
    rocketModuleState = defaultRocketState[1];
    itemState = defaultRocketState[2];
    bufferCopyRocket();
}

void levelNew(void) {
    if(!playerLevel % 4) {
        rocketReset();
        ++playerLives;
        playerInit();
    }
    levelInit();
}

void mainLoop(void) {
    while(true) {
        checkTermination();
        byte funToCall = states[currentState]->utype;
        // should not be needed, just in case while debugging
#ifndef NDEBUG
        funToCall &= 0x1f;
#endif
        mainJumpTable[funToCall]();
        newActor();
    }
}

void newGame(void) {
    srand(time(NULL));
    playerLevel = 0;
    playerLives = 4;
    rocketReset();
    inactivePlayerLevel = 0;
    if(gameOptions.players) {
        inactivePlayerLives = 5;
    } else {
        inactivePlayerLives = 0;
    }
    levelNew();
    currentAlienNumber = 0;
    mainLoop();
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

// update functions
void frameRateLimiter(void){
    static byte lastFrame = 0;
    byte time;
    do {
        time = (byte)getGameTime();
    } while (time == lastFrame);
    lastFrame = time;
}

void jetmanFlyThrust(void) {
}

void sfxPickupItem() {
    playSound(0x30, 0x40);
}

void scoreLabelFlash(Coords coords, bool on) {
    Attrib a = getAttrib(coords.x / 8, coords.y / 8);
    if(a.flash != on) {
        a.flash = on;
        setAttrib(coords.x / 8, coords.y / 8, a);
    }
}


void gamePlayStarts(void){
    if(playerDelayCounter) {
        --playerDelayCounter;
        Coords coords = {
            .x = (currentPlayerNumber ? 0xd8 : 0x18),
            .y = 0 
        };
        if(playerDelayCounter) {
            scoreLabelFlash(coords, true);
        } else {
            sfxPickupItem();
            scoreLabelFlash(coords, false);
        }
    }
    if(!playerDelayCounter)
        jetmanFlyThrust();
}

void jetmanWalk(void){}
void meteorUpdate(void){}
void collisionDetection(void){}
void crossedShipUpdate(void){}
void sphereAlienUpdate(void){}
void jetFighterUpdate(void){}
void animateExplosion(void){}
void rocketUpdate(void){}
void rocketTakeoff(void){}
void rocketLanding(void){}
void sfxEnemyDeath(void){}
void sfxJetmanDeath(void){}
void itemCheckCollect(void){}
void ufoUpdate(void){}
void laserBeamAnimate(void){}
void squidgyAlienUpdate(void){}

byte itemCalcDropColumn(void) {
    return itemDropPositionTable[rand() & 0xf];    
}

static bool invalidJetmanState() {
    return (
        jetmanState.utype == 0 ||
        (jetmanState.utype & 0x3f) >= 3
    );
}

void itemNewFuelPod() {
    if(
        invalidJetmanState() || // impossible?  
        rocketModuleState.type != RMUnused ||
        rocketState.fuelCollected >= 6 ||
        (getGameTime() & 0xf) != 0
    ) {
        return;
    }
    rocketModuleState = defaultRocketModuleState;
    rocketModuleState.x = itemCalcDropColumn();
}
void itemNewCollectible() {
    if(
        invalidJetmanState() ||
        itemState.type != RMUnused ||
        (getGameTime() & 0x7f)
    ) {
        return;
    }
    itemState = defaultItemState;
    itemState.x = itemCalcDropColumn();
    byte r = (rand() & 0xe);
    if(r & 0x8)
        r = 0x8; // 1000 0110 0100 0010 0000 = 8 6 4 2
    r |= 0x20;  // 32 + 8|6|4|2|0
    itemState.jumpTableOffset = r;
}

void newActor(void){
    ++currentState;
    if(currentState > maxState) {  // @FIX self modifying code
        while(IsKeyDown(KEY_LEFT_SHIFT));
        byte r = (byte) rand();
        if(
            (r > 32 || currentAlienNumber > 3) || 
            currentAlienNumber <= 6 ||
            playerDelayCounter == 0 ||
            jetmanState.direction.fly == 0
        )
        {
            for(byte b = 0; b < array_sizeof(alienState); ++b) {
                if(alienState[b].type == RMUnused) {
                    alienState[b] = defaultAlienState;
                    r = (rand() & 1 ? 0x20 : 0);
                    alienState[b].frame = r; //moving
                    alienState[b].utype = r; //direction
                    r = rand() & 0x7f + 0x28;
                    alienState[b].y = r;
                    r = rand() & 0x3 + 2;
                    alienState[b].color = r;
                    alienState[b].jumpTableOffset = (r & 1);
                    r = itemLevelObjectTypes[playerLevel & 0x07];
                    r |= (alienState[b].utype & 0xc0);
                    alienState[b].utype = r;
                    break;
                }
            }
        }
        itemNewFuelPod();
        itemNewCollectible();
        currentState = 0;
    }
}