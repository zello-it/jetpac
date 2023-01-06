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
    ZEROARRAY(bufferAliensRight, Sprite);
    bufferAliensRight[0].data = buffers[0];
    bufferAliensRight[1].data = buffers[1];
    ZEROARRAY(bufferAliensLeft, Sprite);
    bufferAliensLeft[0].data = buffers[2];
    bufferAliensLeft[1].data = buffers[3];
    ZEROARRAY(bufferItems, Sprite);
    bufferItems[0].data = buffers[4];
    bufferItems[1].data = buffers[5];
    bufferItems[2].data = buffers[6];
    bufferItems[3].data = buffers[7];
    bzero(buffers, sizeof(buffers));
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

void bufferCopy(byte* what, Sprite* where, byte height, byte shift) {
    where->xoffset = 0;
    where->width = 3;
    height = (height > 0x11 ? height : 0x10);
    where->height = height;
    byte* ptrto = where->data;
    while(height--) {
        ptrto = writeThreeBytes(what, ptrto, shift);
        what += 2;
    }
}

void bufferCopyRocket(void) {
    // copia quattro volte lo stesso modulo?
    Sprite* sprite = collectibleSpriteTable[4 + playerLevel / 4];
    Sprite* dest = bufferItems;
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
        textOutAttrib((Coords){.x = pos, .y = 0}, buf, a);
        squareOut((Coords){.x = pos + 8, .y = 0}, tileLifeIcon, a);
    } else {
        textOutAttrib((Coords){.x = pos, .y = 0}, "  ", a);
    }
}

void displayAllPlayerLives() {
    displayPlayerLives(0);
    displayPlayerLives(1);
}

void playerInit() {
    jetmanState = defaultPlayerState;
    playerDelayCounter = (gameOptions.players ? Player2Delay : Player1Delay);
    --playerLives;
    displayAllPlayerLives();
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
        Sprite* buf = &bufferAliensRight[b];
        bufferCopy(sprite->data, buf, sprite->height, 0);
    }
    for(byte b = 0; b < 2; ++b) {
        AlienSprite* sprite = alienSpriteTable[index++];
        Sprite* buf = &bufferAliensLeft[b];
        bufferCopy(sprite->data, buf, sprite->height, 0);
    }
}

void levelInit(void) {
    currentState = 0;
    alienBufferInit();
    resetScreen();
    drawPlatforms();
    displayAllPlayerLives();
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

void waitTick() {
    static word tick = 0;
    word newtick;
    do{
        checkTermination();
        newtick = getGameTime();
    }
    while(tick == newtick);
    tick = newtick;
}

void mainLoop(void) {
    while(true) {
        waitTick();
        byte funToCall = states[currentState]->utype;
        // should not be needed, just in case while debugging
#ifndef NDEBUG
        funToCall &= 0x1f;
#endif
        printf("Calling %d\n", funToCall);
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
    static char buf[7]; // max score is 999999
    snprintf(buf, 7, "%06d", score);
    textOutAttrib((Coords){.x = col * 8, .y = line}, buf, (Attrib){.ink = 7, .bright = 1});
}

void addPointsToScore(byte b) {
    static byte xpos[] = {1, 25};
    uint32_t* score = (currentPlayerNumber ? &p2Score : &p1Score);
    *score += b;
    if(*score > 999999) 
        *score = 999999;
    showScore(1, xpos[currentPlayerNumber], *score);
}

void resetScreen(void){
    clearScreen((Attrib){.ink = 7, .paper = 0, .bright = 1}); 
    textOutAttrib((Coords){.x = 24, .y = 0}, "1UP", (Attrib){.attrib = 0x47});
    textOutAttrib((Coords){.x = 120,.y = 0}, "HI", (Attrib){.attrib = 0x45});
    textOutAttrib((Coords){.x = 216, .y = 0}, "2UP", (Attrib){.attrib = 0x47});
    for(byte row = 0; row < 32; ++ row) {
        setAttrib(row, 1, (Attrib){.attrib = 0x46});
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
    // do nothing
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


Coords getSpritePosition(Sprite* sprite, Coords coords) {
    Coords ret;
    ret.x = sprite->xoffset + coords.x;
    ret.y = coords.y;
    actor.height = sprite->height;
    return ret;
}

void do_maskSprite(byte* spritedata, Coords coords, byte height, byte width) {
    if(height) {
        --height;
        byte* ptr = spritedata;
        Coords c = coords;
        for(byte h = 0; h < height; ++h) {
            for(byte b = 0; b < width; ++b) {
                byte d = ~(*ptr++);
                byteOut(c, d, AND);
                c.x += 8;
            }
            c.x = coords.x; 
            if(--c.y > 192) c.y = 191;
        }
    }
}

void maskSprite(byte* spritedata, Coords coords, byte maskheight, byte spriteheight, byte width) {
    do_maskSprite(spritedata, coords, maskheight, width);
    do_maskSprite(spritedata, coords, spriteheight, width);
}

Sprite* actorGetSpriteAddress(byte x, byte header) {
    if(header & 0x40) {
        x |= 0x8;
    }
    --header;
    byte index = (header << 4) & 0xf0 | x;
    index >>= 1; // word to index
    return spriteTable[index];
}

Sprite* actorFindPosDir(void) {
    return actorGetSpriteAddress(actor.x & 0x06, actor.spriteIndex);
}

/**
 * Update actor tmp buffer with position and sprite index of the 
 * current object
*/
void actorUpdatePosDir(State* cur) {
    actor.x = cur->x;
    actor.y = cur->y;
    actor.spriteIndex = cur->spriteIndex;
}

void actorEraseDestroyed(State* state, Sprite* sprite, Coords coords) {
    byte c = actor.height;
    actor.spriteHeight = 0;
    actor.height = 0;
    maskSprite(sprite->data, coords, c, 0, actor.width);
}

Coords actorFindDestroy(State* state){
    Sprite* sprite = actorFindPosDir();
    Coords coords = getSpritePosition(sprite, actor.coords);
    actorEraseDestroyed(state, sprite, coords);
    return coords;
}

void actorUpdate(State* state, Sprite* sprite) {
    actorCoords.x = state->x + sprite->xoffset;
    actorCoords.y = state->y;
    actor.width = sprite->width;
    actor.height = actor.spriteHeight = sprite->height;
}

byte actorUpdateSize(Sprite* sprite, Coords coords){
    byte ret = actor.height;
    byte spriteHeight = actor.spriteHeight;
    if(!ret&& !spriteHeight)
        return ret;
    actor.height = 0;
    actor.spriteHeight = 0;
    maskSprite(sprite->data, coords, ret, spriteHeight, actor.width); // this will recall into actorUpdateSize...
    return ret;
}

void actorEraseMovedSprite(State* state, Sprite* sprite, Coords coords) {
    sbyte diff = actor.y - state->y;
    if(diff == 0) {
        actorUpdateSize(sprite, coords);
        return;
    }
    if(diff > 0) {
        if(actor.height > diff) {
            actorUpdateSize(sprite, coords);
            return;
        }
        actor.height -= diff;
        maskSprite(sprite->data, coords, diff, actor.height, actor.width);
    } else {
        diff = byteAbs(diff);
        if(actor.spriteHeight < diff)
            actorUpdateSize(sprite, coords);
        else{
            actor.spriteHeight -= diff;
            maskSprite(sprite->data, coords, diff, actor.spriteHeight, actor.width);
        }
    }
}

void actorUpdatePosition(State* state, Sprite* sprite) {
    Coords coords = getSpritePosition(sprite, actor.coords);
    actorUpdate(state, sprite);
    actorEraseMovedSprite(state, sprite, coords);
}


Sprite* findActorSpriteAndUpdate(State* state) {
    Sprite* sprite = actorGetSpriteAddress(state->x & 0x6, state->spriteIndex);
    actorUpdate(state, sprite);
    return sprite;
}

void updateAndEraseActor(State* state) {
    Sprite* s = findActorSpriteAndUpdate(state);
    Sprite* spriteActor = actorFindPosDir();
    Coords act = getSpritePosition(spriteActor, actorCoords);
    actorEraseMovedSprite(state, spriteActor, actorCoords);
}

bool alienCollision(State* state) {
    bool ret = false;
    if(!jetmanState.direction.fly) {
        sbyte distance = byteAbs(jetmanState.x - state->x);
        if(distance < 12) {
            distance = jetmanState.y - state->y;
            sbyte thres = 21;
            if(distance < 0) {
                thres = state->height;
                distance = byteAbs(distance);
            }
            ret = (distance < thres);
        }
    }
    return ret;
}

void colorizeSprite(State* state) {
   Attrib a = {.attrib = state->color};
   for(byte w = 0; w < actor.width; ++w ) {
    byte col = (actorCoords.x + w) / 8;
    for(byte h = 0; h < (actor.height + 4) / 8; ++h) {
        setAttrib(col, (actorCoords.y - h) / 8, a);
    }
   } 
}

byte jetmanPlatformCollision(State* state) {
    byte eRet;
    for(byte b = 0; b < 4; ++b) {
        eRet = 0;
        GFXParams* platform = &gfxParamsPlatforms[b];
        sbyte diff = platform->x - state->x;
        if(diff >= 0) {
            // L75D1
            if((state->spriteIndex & 0x1f) ==0)
            {
                if(diff >= 9)
                    diff -= 9;
            }
        }
        else {
            diff = byteAbs(diff);
            eRet |= 0x40;
        }
        if(diff > platform->width) {
            continue;
        }
        diff += 0x12;
        if(diff >= platform->width) {
            eRet |= 8;
        }
        diff = state->y - platform->y + 2;
        if(diff < 0) // no hit
            continue;
        if(diff < 2) {// hit from above, Y_2
            eRet = 0x84;
            break;
        } else if(diff < state->height) { // hit from below
            eRet |= 0x4;
            break;
        } else if((diff - 2) > state->height) {
            eRet |= 0x14;
            break;
        }
    }
    return eRet;
}

byte laserBeamFire(State* state) {
    byte cRet = 0;
    for(byte b = 0; b < array_sizeof(laserBeamParam);  ++b) {
        LaserBeam* beam = &laserBeamParam[b];
        if(beam->used == LBUnused) {
            byte a = beam->x[1];
            if(a & 0x4){
                sbyte diff = (a & 0xf8) - state->x;
                byte c = (diff >= 0? 0x20 : 0x08);
                diff = byteAbs(diff); 
                if(diff < c) {
                    diff = state->y - beam->y;
                    if(diff >= 0) {
                        diff += 0xc;
                        if(diff < state->height) {
                            cRet = 1;
                            beam->x[0] &= 0xf8;
                            break;
                        }
                    }
                }
            }

        }
    }
    return cRet;
}

void enableAnimationState(State* state, byte value) {
    state->animByte2 = value;
    state->spriteIndex = Animating;
    state->frame = 0;
}

void animationStateReset(State* state) {
    byte tmp = state->spriteIndex & 0xc0 | 0x3;
    state->animByte = 0;
    enableAnimationState(state, tmp);
}

void sfxSetExplodeParam(State* state, byte param) {
    explosionSfxParams = explosionSfxDefaults[param];
}

void explosionAfterKill() {
    jetmanExplodingAnimState.x = jetmanState.x;
    jetmanExplodingAnimState.y = jetmanState.y;
    jetmanExplodingAnimState.animByte = 1;
    enableAnimationState(&jetmanExplodingAnimState, jetmanState.spriteIndex);
}

void alienCollisionAnimSfx(State* state) {
    animationStateReset(state);
    sfxSetExplodeParam(state, 1);
    explosionAfterKill();
}

void do_updateRocketColor(State* state, byte counter){
    while(counter--) {
        colorizeSprite(state);
        actorCoords.y -= 8;
    }
}

void updateRocketColor(State* state) {
    byte base = (playerLevel / 2) & 3;
    byte offset = 0;
    Coords coords = {.x = state->x, .y = state->y};
    for(byte b = 0; b < state->frame; --b) {
        byte idx = base | offset;
        Sprite* sprite = collectibleSpriteTable[idx];
        //coords = getSpritePosition(sprite, actorCoords);
        actorUpdatePosition(state, sprite);
        state->y -= 0x10;
        actor.y -= 0x10;
        offset += 2; // era 4, ma si trattava di words
    }
    actor.width = 2;
    actor.height = 0;
    state->y = coords.y; 
    actorCoords = coords;
    Attrib attr;
    byte counter;
    byte a = state->frame << 1;
    if(a >= 6 && state->fuelCollected != 0){
        attr.attrib = (byte)getGameTime() & 0x4 | 0x43;
    } else
        attr.attrib = 0x47;
    if(state->fuelCollected < 6) {
        counter = state->fuelCollected;
        state->color = 0x43;
        do_updateRocketColor(state, counter);
        counter = 6 - state->fuelCollected;
        attr.attrib = 0x47;
    }
    state->color = attr.attrib;
    do_updateRocketColor(state, counter);
}

void jetmanWalk(void){}
void meteorUpdate(void){
    State* cur = states[currentState];
    actorUpdatePosDir(cur);
    ++currentAlienNumber;
    byte x = cur->x;
    if((cur->frame & 0x40) != 0){
        x -= cur->xspeed;
    }
    else{
        x += cur->xspeed;
    }
    cur->x = x;
    cur->y += cur->yspeed;
    updateAndEraseActor(cur);
    colorizeSprite(cur);
    if(jetmanPlatformCollision(cur) & 0x4) {
        // MeteorUpdate2
        animationStateReset(cur);
        sfxSetExplodeParam(cur, 0);
    } else {
        if(laserBeamFire(cur) != 0){
            // increase score and display: MeteorUpdate1
            addPointsToScore(25);
        } else {
            if(alienCollision(cur) == 0) {
                // no points  and die
                alienCollisionAnimSfx(cur);
            }
        }
    }

}
void collisionDetection(void){}
void crossedShipUpdate(void){}
void sphereAlienUpdate(void){}
void jetFighterUpdate(void){}
void animateExplosion(void){}

void rocketUpdate(void){
    State* cur = states[currentState];
    actorUpdatePosDir(cur);
    // to do
    if(alienCollision(cur) || rocketState.fuelCollected < 6) {
        updateRocketColor(cur);
    }
    else { // level finished
        ++rocketState.utype;
        actorUpdatePosDir(&jetmanState);
        actorFindDestroy(&jetmanState);
        jetmanState.utype = 0;
        ++playerLives;
        displayAllPlayerLives();
    }
}
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