#include "game.h"
#include "data.h"
#include "video.h"
#include "menu.h"
#include "sprite.h"
#include "keyboard.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ZEROSTRUCT(name, type) (name = (const type){0})
#define ZEROARRAY(name, type) {for(int n=0; n < array_sizeof(name); ++n){ZEROSTRUCT(name[n], type);}}

void frameRateLimiter(State* state);
void gamePlayStarts(State* state);
void jetmanWalk(State* state);
void meteorUpdate(State* state);
void collisionDetection(State* state);
void crossedShipUpdate(State* state);
void sphereAlienUpdate(State* state);
void jetFighterUpdate(State* state);
void animateExplosion(State* state);
void rocketUpdate(State* state);
void rocketTakeoff(State* state);
void rocketLanding(State* state);
void sfxEnemyDeath(State* state);
void sfxJetmanDeath(State* state);
void itemCheckCollect(State* state);
void ufoUpdate(State* state);
void laserBeamAnimate(State* state);
void squidgyAlienUpdate(State* state);

typedef void(*Fun)(State*);
Fun mainJumpTable[] = {
    frameRateLimiter,       //0
    gamePlayStarts,         
    jetmanWalk,
    meteorUpdate,
    collisionDetection,     //4
    crossedShipUpdate,
    sphereAlienUpdate,
    jetFighterUpdate,
    animateExplosion,       //8
    rocketUpdate,
    rocketTakeoff,
    rocketLanding,
    sfxEnemyDeath,          //12 0xc
    sfxJetmanDeath,
    itemCheckCollect,
    ufoUpdate,
    laserBeamAnimate,       //16 0x10 
    squidgyAlienUpdate
};

bool gameReset = false;
byte flipped[256];

void newActor(void);
void resetScreen(void);
void jetmanRedraw(State* cur);
byte checkPlatformCollision(State* cur);
void actorSaveSpritePos(State* state);

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


#ifndef NDEBUG
static inline void bin(byte s) {
    char buf[9] = {0};
    for(int i = 0; i < 8; ++i) {
        buf[7 - i] = (s & (1 << i) ? '1' : '0');
    }
    buf[8] = '\0';
    printf("%s", buf);
}

void dbgPrint(const char * arg, byte b) {
    printf("%s ", arg);
    bin(b);
    printf("\n");
}
#endif

byte* writeThreeBytes(byte* what, byte* to, byte shift, byte flip) {
    byte b1, b2, b3;
    b1 = *what++;
    b2 = *what++;
    b3 = 0;

    while(shift--) {
        byte rep = (b1 & 1) << 7;
        b1 >>= 1;
        byte rep2 = (b2 & 1) << 7;
        b2 = (b2 >> 1) + rep;
        b3 = (b3 >> 1) + rep2;
    }
    if(flip) {
        swap(&b1, &b3);
        b1 = flipped[b1];
        b2 = flipped[b2];
        b3 = flipped[b3];
    }
    *to++ = b1;
    *to++ = b2;
    *to++ = b3;
    return to;
}

void bufferCopy(byte* what, Sprite* where, byte height, byte shift, byte flipped) {
    where->xoffset = 0;
    where->width = 3;
    height = (height > 0x11 ? height : 0x10);
    where->height = height;
    byte* ptrto = where->data;
    while(height--) {
        ptrto = writeThreeBytes(what, ptrto, shift, flipped);
        what += 2;
    }
}

void bufferCopyRocket(byte arg) {
    byte idx = ((playerLevel / 2) & 0x6 | arg) / 2;
    Sprite* dest = bufferItems;
    rocketModAttached = 2;
    jetmanRocketModConnected = 0;
    for(byte b = 0; b < 4; ++b) {
        Sprite* sprite = collectibleSpriteTable[idx];
        bufferCopy(sprite->data, dest + b, sprite->height, 2 * b, false);
    }
}

void updateHiScore(){
    uint32_t maxscore = (p1Score > p2Score ? p1Score: p2Score);
    if(hiScore < maxscore)
        hiScore = maxscore;
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
    displayPlayerLives(Player1);
    displayPlayerLives(Player2);
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
            byte counter = (platform->width >> 2) - 4;
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
//    jetmanRocketModConnected = 0;
    byte index = ((playerLevel * 4) & 0x1c) >> 1;
    printf("Idx buffer is %d\n", index);
    for(byte b = 0; b < 2; ++b) {
        AlienSprite* sprite = alienSpriteTable[index+b];
        Sprite* buf = &bufferAliensRight[b];
        bufferCopy(sprite->data, buf, sprite->height, b * 4, false);
    }
    jetmanRocketModConnected = 1;
    for(byte b = 0; b < 2; ++b) {
        AlienSprite* sprite = alienSpriteTable[index+b];
        Sprite* buf = &bufferAliensLeft[b];
        bufferCopy(sprite->data, buf, sprite->height, b * 4, true);
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
    bufferCopyRocket(8);
}

void levelNew(void) {
    if(!playerLevel % 4) {
        rocketReset();
        ++playerLives;
        playerInit();
    }
    levelInit();
}

bool checkTick() {
    static word tick = 0;
    bool ticked = false;
    word newtick = getGameTime();
    checkTermination();
    //gameSleep(3); // rallentiamo il gioco
    if(tick != newtick) {
        tick = newtick;
        ticked = true;
    }
    return ticked;
}

void mainLoop(void) {
    currentState = 0;
    while(!gameReset) {
        if(checkTick())
        {
            di();
            mainJumpTable[jetmanState.spriteIndex & 0x3f](&jetmanState);
            ei();
        }
        byte funToCall = states[currentState]->spriteIndex & 0x3f;
        // should not be needed, just in case while debugging
#ifndef NDEBUG
//        printf("Calling %d on sprite idx %d\n", funToCall, currentState & 0x3f);
#endif
        mainJumpTable[funToCall](states[currentState]);
        newActor();
    }
}

void initFlipped() {
    for(word b = 0; b < 256; ++b) {
        byte res = 0;
        for(byte bit = 0; bit < 8; ++bit){
            res <<= 1;
            res |= ((b & (1 << bit)) != 0);
        }
        flipped[b] = res;
    }
}

void newGame(void) {
    srand(time(NULL));
    maxState = 0x0f;
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
    showScore(8, xpos[currentPlayerNumber ? 1 : 0], *score);
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
    for(int col = 0; col < 0x20; ++col) {
        setAttrib(col, 1, (Attrib){.attrib = 0x46});
    }
}

void startGame(void) {
    initFlipped();
    while(true) {
        resetGlobals();
        resetScreen();
        jetmanSpeedModifier = 0x4;
        menuScreen();
        newGame();
        gameReset = false;
    }
}

// update functions
void frameRateLimiter(State* cur){
    gameSleep(5000);
    // do nothing
}

void jetmanLands(State* cur) {
    cur->moving.ud = 0;
    cur->spriteIndex = (cur->spriteIndex & 0xc0) | 0x2;
    cur->xspeed = 0;
    cur->yspeed = 0;
    jetmanRedraw(cur);
}

void jetmanCollision(State* cur) {
    byte eRes = checkPlatformCollision(cur);
    if(!(eRes & 0x4)){
        jetmanRedraw(cur);
        return;
    }
    if(eRes & 0x80) {
        jetmanLands(cur);
        return;
    }
    if(eRes & 0x10) {
        cur->moving.ud = 1;
        jetmanRedraw(cur);
        return;
    }
    eRes = (eRes ^ 0x40) & 0x40;
    cur->umoving = (cur->umoving & 0xbf | eRes);
    jetmanRedraw(cur);
}

void jetmanFlyVertical(State* cur) {
    word speedy = 8 * (word)(cur->yspeed);
    word y = ((word)(cur->y) << 8) | actor.ySpeedDecimal;
    if(cur->moving.ud)
        y += speedy;
    else
        y = y - speedy;
    actor.ySpeedDecimal = (byte) y;
    cur->y = (y >> 8);
    if(cur->y >= 0xc0){ 
        cur->moving.ud = 0;
    } else if(cur->y < 0x2a) {
        cur->moving.ud = 1;
        byte a = cur->yspeed >> 1;
        if(a) {
            cur->yspeed = a;
        }
    }
    jetmanCollision(cur);
}

void jetmanDirFlipY(State* cur) {
    sbyte speed = jetmanSpeedModifier - 8 + cur->yspeed;
    if(speed >= 0) {
        cur->yspeed = speed;
    } else {
        cur->yspeed = 0;
        cur->moving.ud = ~cur->moving.ud;
    }
    jetmanFlyVertical(cur);
}

void jetmanFlyHorizontal(State* cur) {
    word xspeed = (word)(cur->xspeed) * 8;
    word xpos = (cur->x << 8) | actor.xSpeedDecimal;
    if(cur->moving.rl) {
        xpos -= xspeed;
    } else {
        xpos += xspeed;
    }
    actor.xSpeedDecimal = (byte) (xpos & 0x00ff);
    cur->x = (byte)((xpos & 0xff00) >> 8);
    if(readInputHover() == Hover) {
        cur->yspeed = 0;
        jetmanFlyVertical(cur);
    }
    if(readInputThrust() != Thrust) {
        //    jetmanSetMoveDown(cur);
        cur->spriteIndex |= 0x80;
        if(cur->moving.ud) {
            //jetmanSpeedIncY
            byte speed = cur->yspeed + 8 - jetmanSpeedModifier;
            cur->yspeed = umin(speed, 0x3f); //3f?
            jetmanFlyVertical(cur);
        } else {
            jetmanDirFlipY(cur);
        }
    } else {
        cur->spriteIndex &= ~0x80;
        if(cur->moving.ud) {
            jetmanDirFlipY(cur);
            return;
        }
        sbyte yspeed = 0x8 - jetmanSpeedModifier + cur->yspeed;
        if(yspeed > 0x3f){
            yspeed = 0x3f;
        }
        cur->yspeed = yspeed;
        jetmanFlyVertical(cur);
    }
}

void jetmanDirFlipX(State* cur) {
    sbyte xspeed = jetmanSpeedModifier - 0x8 + cur->xspeed;
    if(xspeed < 0) {
        cur->xspeed = 0;
        cur->moving.rl = ~cur->moving.rl;
    } else {
        cur->xspeed = xspeed;
    }
    jetmanFlyHorizontal(cur);
}
void jetmanFlyIncSpdX(State* cur) {
    sbyte xspeed = -jetmanSpeedModifier;
    xspeed += (0x8 + cur->xspeed);
    cur->xspeed = umin(0x40, xspeed);
    jetmanFlyHorizontal(cur);
}

void jetmanFlyThrust(State* cur) {
    actorSaveSpritePos(cur);
    enum KeyboardResult res = readInputLR();
    switch(res) {
        case Right:
            cur->spriteIndex &= ~0x40;
            if(cur->moving.rl){
                jetmanDirFlipX(cur);
            } else {
                jetmanFlyIncSpdX(cur);
            }
            return;
        case Left:
            cur->spriteIndex |= 0x40;
            if(cur->moving.rl) {
                jetmanFlyIncSpdX(cur);
            } else {
                jetmanDirFlipX(cur);
            }
            return;

        case None:
            {
                if(!(getGameTime() & 0x1)) {
                    jetmanFlyHorizontal(cur);
                } else {
                    sbyte speed = jetmanSpeedModifier - 8 + cur->xspeed;
                    if(speed < 0) {
                        speed = 0;
                    }
                    cur->xspeed = speed;
                    jetmanFlyHorizontal(cur);
                }
            }
        default:
            break; // to skip a warning, it's unreacheable code...
    }
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


void gamePlayStarts(State* cur){
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
        jetmanFlyThrust(cur);
}


bool collisionWithJetman(State* state) {
    bool ret = false;
    byte jstate = jetmanState.spriteIndex & 0x3f;
    if(jstate == 1 || jstate == 2 ) {
        byte distance = byteAbs(jetmanState.x - state->x);
        if(distance < 0xc) {
            distance = jetmanState.y - state->y;
            sbyte thres = 0x15;
            if((sbyte)distance < 0) {
                thres = state->height;
                distance = byteAbs(distance) + 0xe;
            }
            ret = (distance < thres);
        }
    }
    return ret;
}

byte checkPlatformCollision(State* state) { //jetmanPlatformCollision => ok!
    byte eRet = 0;
    for(byte b = 0; b < 4; ++b) {
        eRet = 0;
        GFXParams* platform = &gfxParamsPlatforms[b];
        sbyte diff = platform->x - state->x;
        if(diff >= 0) {
            // L75D1
            if((state->spriteIndex & 0x3f) == 3 && diff >=9)
            {
                diff -= 9;
            }
        }
        else {
            diff = byteAbs(diff);
            eRet |= 0x40;           // platform is on the right
        }
        if((byte)diff >= platform->width) {   // we don't hit (platform->x is coord right?)
            continue;
        }
        diff += 0x12;
        if((byte)diff >= platform->width) {
            eRet |= 8;
        }
        diff = state->y - platform->y + 2;
        if(diff < 0) // no hit
            continue;
        if(diff < 2) {// hit from above, Y_2
            eRet |= 0x84;
            break;
        } else if(diff < state->height) { // hit from below
            eRet |= 0x4;
            break;
        } else if((diff - 2) < state->height) {
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
        if(beam->used != LBUnused) {
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
    state->oldSpriteIndex = value;
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
    jetmanState.spriteIndex = 0;
}

void alienCollisionAnimSfx(State* state) {
    animationStateReset(state);
    sfxSetExplodeParam(state, 1);
    explosionAfterKill();
}

static inline void swapState(State* one, State* two) {
    State tmp = *one;
    *one = *two;
    *two = tmp;
}

void playersSwap() {
    swap(&playerLevel, &inactivePlayerLevel);
    swapState(&rocketState, &inactiveRocketState[0]);
    swapState(&rocketModuleState, &inactiveRocketState[1]);
    swapState(&itemState, &inactiveRocketState[2]);
}

void resetGame() {
    gameReset = true;
}

void displayGameOver() {
    static char gameover[] = "GAME OVER PLAYER X";
    static const Coords coords = {.x = 0x38, .y = 70};
    gameover[strlen(gameover) - 1] = (currentPlayerNumber ? '2' : '1');
    resetScreen();
    textOutAttrib(
        coords, gameover,(Attrib){.attrib = 0x47}
    );
    gameSleep(1.0e6);
}

void playerTurnEnds(State* state) {
#define IDXFROM 7
#define IDXTO  maxState
   
    for(byte b = IDXFROM; b <= IDXTO; ++b) {
        states[b]->spriteIndex = 0;
    }
    rocketModuleState.state &= ~2;
    itemState.state &= ~2;
    printf("rocketmodstate %x, itemstate %x\n", rocketModuleState.state, itemState.state);
    if(gameOptions.players) {
        if(inactivePlayerLives) {
            if(!playerLives) {
                displayGameOver();
            }
            playersSwap();
            currentPlayerNumber = ~currentPlayerNumber;
            byte offset = ((rocketState.state) << 3) & 0x38;
            bufferCopyRocket(offset);
            levelInit();
            playerInit();
            return;
        }
    }
    if(!playerLives) {
        updateHiScore();
        displayGameOver();
        resetGame();
    } else {
        levelInit();
        playerInit();
    }

}

void do_updateRocketColor(State* state, byte counter){
    while(counter--) {
        colorizeSprite(state);
        actorCoords.y -= 8;
    }
}

void updateRocketColor(State* state) {
    byte base = (playerLevel / 4) & 3;
    byte offset = 0;
    Coords coords = {.x = state->x, .y = state->y};
    for(sbyte b = state->frame; b > 0; --b) {
        byte idx = base | offset;
        Sprite* sprite = collectibleSpriteTable[idx];
        //coords = getSpritePosition(sprite, actorCoords);
        actorUpdatePosition(state, sprite);
        state->y -= 0x10;
        actor.y -= 0x10;
        offset += 4;
    }
    actor.width = 2;
    actor.height = 0;
    state->y = coords.y; 
    actorCoords = coords;
    Attrib attr;
    byte counter = state->frame * 2;
    if(counter >= 6 && state->fuelCollected != 0){
        attr.attrib = ((byte)getGameTime() >> 2) & 0x4 | 0x43;
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

void itemDropGoldBar(State* cur){
    cur->color = 0x46;
    itemDrawSprite(cur);
}
void itemDropChemical(State* cur){
    if((getGameTime() & 0x1f) < 18) {
        cur->color = 0;
    }
    else {
        cur->color = 0x45;
    }
    itemDrawSprite(cur);
}
void itemDropPlutonium(State* cur){
    cur->color = 0x44;
    itemDrawSprite(cur);
}
void itemDropRandomColor(State* cur){
    byte col = getGameTime();
    col >>= 2;
    col &= 7;
    if(!col) 
        ++col;
    col |= 0x40;
    cur->color = col;
    itemDrawSprite(cur);
}

void itemDropNew(State* cur) {
    typedef void(*DropFun)(State*);
    DropFun dropFun[] = {
        itemDropGoldBar,
        itemDropChemical,
        itemDropChemical,
        itemDropPlutonium,
        itemDropRandomColor
    };
    dropFun[(cur->jumpTableOffset & 0xf) / 2](cur);
}



void sfxLaserFire() {
    byte dur = 0x08;
    while(dur++ < 0x38)
        playSound(dur, dur);
}

void laserBeamDraw(LaserBeam* laser, byte x) {
    laser->y = jetmanState.y - 0x0d;
    laser->x[0] = x;
    x &= 0xfb;
    for(byte b = 1; b < 4; ++b) {
        laser->x[b] = x;
    }
    laser->length = (byteRand() & 0x38) | 0x84;
    laser->color = laserBeamColors[byteRand() & 0x3];
    sfxLaserFire();
}

byte laserBeamShootRight(LaserBeam* laserBeam, byte arg) {
    byte x = jetmanState.x & 0x7;
    if(x) {
        arg += 8;
    }
    arg += 0x10;
    arg &= 0xfe;
    return arg;
}

void laserBeamInit(LaserBeam* laserBeam) {
    laserBeam->used = LBUsed;
    byte dir = jetmanState.spriteIndex;
    byte x = (jetmanState.x & 0xf8) | 0x5;
    if((dir & 0x40) == 0) {
        x = laserBeamShootRight(laserBeam, x);
    } else {
        x -= 8;
    }
    laserBeamDraw(laserBeam, x);
}

void laserNewIfFreeSlot(State* cur) {
    if(getGameTime() & 3)
        return;
    for(LaserBeam* laser = laserBeamParam; laser < laserBeamParam + array_sizeof(laserBeamParam); ++laser) {
        if(laser->used == LBUnused) {
            laserBeamInit(laser);
            break;
        }
    }
}

void jetmanRedraw(State* cur) {
    updateAndEraseActor(cur);
    colorizeSprite(cur);
    enum KeyboardResult res = readInputFire();
    if(res == Fire) {
        laserNewIfFreeSlot(cur);
    }
}

void jetmanWalkOffPlatform(State* cur) {
    cur->spriteIndex = cur->spriteIndex & 0xc0 | 1;
    State* thruster = &jetmanThrusterAnimState;
    if(!thruster->spriteIndex) {
        thruster->spriteIndex = 0x03;
        thruster->x = jetmanState.x;
        thruster->y = jetmanState.y;
        animationStateReset(thruster);
    }
    cur->y -= 2;
    jetmanRedraw(cur);
}

void jetmanWalk(State* cur){
    actorSaveSpritePos(cur);
    enum KeyboardResult res = readInputLR();
    switch(res) {
        case Right:
            {
                ++cur->x;
                cur->spriteIndex &= ~0x40;
                cur->moving.rl = 0;
                cur->xspeed = 0x20;
            }
            break;
        case Left:
            {
                --cur->x;
                cur->spriteIndex |= 0x40;
                cur->moving.rl = 1;
                cur->xspeed = 0x20;
            }
            break;
        default:
            cur->xspeed = 0;
            break;
    }
    res = readInputThrust();
    byte eRes = checkPlatformCollision(cur);
    if(res == Thrust || !(eRes & 0x4)) {
        jetmanWalkOffPlatform(cur);
        return;
    }
    if((eRes & 0x8) == 0 || cur->xspeed != 0) {
        jetmanRedraw(cur);
        return;
    }
    if(cur->spriteIndex & 0x40) {
        --cur->x;
    } else {
        ++cur->x;
    }
    cur->xspeed = 0x20;
    jetmanRedraw(cur);
}
void meteorUpdate(State* cur){
    actorSaveSpritePos(cur);
    ++currentAlienNumber;
    if(cur->moving.rl){
        cur->x -= cur->xspeed;
    }
    else{
        cur->x += cur->xspeed;
    }
    cur->y += cur->yspeed; 
    updateAndEraseActor(cur);
    colorizeSprite(cur);
    if(checkPlatformCollision(cur) & 0x4) {
        // MeteorUpdate2
        animationStateReset(cur);
        sfxSetExplodeParam(cur, 0);
    } else {
        if(laserBeamFire(cur) != 0){
            // increase score and display: MeteorUpdate1
            addPointsToScore(25);
            animationStateReset(cur);
            sfxSetExplodeParam(cur, 0);
        } else {
            if(collisionWithJetman(cur) == true) {
                // no points  and die
                alienCollisionAnimSfx(cur);
            }
        }
    }
}

void sfxRocketBuild() {
    playSound(0x20, 0x50);
}
void sfxThrusters() {
    
}

void pickupRocketItem(State* cur) {
    byte a = cur->jumpTableOffset;  
    if(a == 0x18) {             // fuel pod
        if(cur->y < 0xb0) {
            cur->y += 2;
            redrawSprite(cur);
            return;
        } else {
            ++rocketState.fuelCollected;
        }
    } else {
        a <<= 1;
        a += cur->y;
        if(a < 0xb7) {
            cur->y += 2;
            redrawSprite(cur);
            return;
        } else {
            rocketModuleState.state |= 1;
            ++rocketState.state;
            bufferCopyRocket(cur->jumpTableOffset + 0x8);
        }
    }
    actorFindDestroy(cur);
    cur->spriteIndex = 0;
    sfxRocketBuild();
}

void carryRocketItem(State* cur){
    cur->x = jetmanState.x;
    cur->y = jetmanState.y;
    byte diff = byteAbs(rocketState.x - cur->x);
    if(diff < 6) {
        cur->state |= Drop;
        cur->x = rocketState.x;
    }
    redrawSprite(cur);
}

void sfxPickupFuel() {
    playSound(0x50, 0x28);
}

void collectRocketItem(State* cur) {
    cur->state |= Carrying;
    actorFindDestroy(cur);
    addPointsToScore(100);
    sfxPickupFuel();
    cur->x = jetmanState.x;
    cur->y = jetmanState.y;
    actorSaveSpritePos(cur);
    redrawSprite(cur);
}

void collisionDetection(State* cur){
    actorSaveSpritePos(cur);
    enum RMState curstate = cur->state;
    if(curstate & Drop) {
        pickupRocketItem(cur);
    } 
    else if (curstate & Carrying) {
        carryRocketItem(cur);
    }
    else if(!(curstate & Free)) {
        getCollectibleID(cur);
    }
    else {
        if(collisionWithJetman(cur) == true){
            collectRocketItem(cur);
        }
        if((checkPlatformCollision(cur) & 0x4) == 0)
            cur->y += 2;
        redrawSprite(cur);
    }
}
void crossedShipUpdate(State* cur){}
void sphereAlienUpdate(State* cur){}

void jetFighterUpdateX(State* cur, byte data) {
    // aka jfu3
    byte a = (data + getGameTime()) & 0x7f | 0x20;
    cur->moving.jet_moving = 1;
    cur->xspeed = a;
    cur->color = 0x47;
}

void destroyFighter(State* cur) {
   addPointsToScore(55);
   sfxThrusters();
   animationStateReset(cur);            
}

void jetFighterUpdate(State* cur){
    ++currentAlienNumber;
    actorSaveSpritePos(cur);
    actor.spriteIndex = (cur->spriteIndex & 0xc0) | 0x3;
    byte y = 0, x = 0;
    if(cur->moving.jet_moving){
        if(--cur->xspeed == 0) {
            return destroyFighter(cur);
        }
        x = (cur->spriteIndex & 0x40 ? -4 : 4);
        if(jetmanState.y >= cur->y) 
            y = 0x2;
        else 
            y = -0x2;
    } else {
        byte a = byteRand() & 0x1f;
        if(a == 0) {
            jetFighterUpdateX(cur, 0);
        }
        a = jetmanState.y - 0xc;
        if(a == cur->y) {
            jetFighterUpdateX(cur, a);
        }
    }
    if(getGameTime() & 0x40) {
        y = 0x2;
    } else {
        y = -0x2;
    }
    cur->spriteIndex = (cur->spriteIndex & 0xc0) | 0x03;
    cur->x += x;
    cur->y += y;
    updateAndEraseActor(cur);
    colorizeSprite(cur);
    if(cur->y < 0x28 || 
        checkPlatformCollision(cur) & 0x4 ||
        laserBeamFire(cur) == 1
    ) {
        return destroyFighter(cur);
    }
    if(collisionWithJetman(cur) == true)
    {
        alienCollisionAnimSfx(cur);
        return;
    }
    cur->spriteIndex = cur->spriteIndex & 0xc0 | 0x07;
}

void animateExplosion(State* cur){
    ++currentAlienNumber;
    byte oldframe = cur->frame;
    if(((byte)getGameTime() & cur->animByte) == 0)
        ++cur->frame;
    Sprite* explosionFrame = explosionSpriteTable[oldframe];

    if(oldframe < 6) {
        SpriteData sd = {
            .spritedata = explosionFrame->data,
            .height = explosionFrame->height,
            .coords = (Coords){.x = cur->x, .y = cur->y},
            .width = explosionFrame->width
        };
        actorSaveSpritePos(cur);
        if(oldframe < 3) {
            maskSprite(NULL, &sd);
            byte r = byteRand() & 0x07 | 0x42;
            cur->color = r;
            colorizeSprite(cur);
        } else {
            maskSprite(&sd, NULL);
        }
    }
    else {
        cur->spriteIndex = cur->oldSpriteIndex;
        actorSaveSpritePos(cur);
        actorFindDestroy(cur);
        cur->spriteIndex = 0;
        if((cur->oldSpriteIndex & 0x3f) < 3) {
            playerTurnEnds(cur);
        }
    }
}

void rocketUpdate(State* cur){
    actorSaveSpritePos(cur);  // update actor with x, y e spriteIndex
    if(collisionWithJetman(cur) == false || rocketState.fuelCollected < 6) {
        updateRocketColor(cur);
    }
    else { // level finished
        ++rocketState.spriteIndex;
        actorSaveSpritePos(&jetmanState);
        actorFindDestroy(&jetmanState);
        jetmanState.spriteIndex = 0;
        ++playerLives;
        displayAllPlayerLives();
    }
}



void rocketAnimateFlames(State* cur) {
    cur->y += 0x15;
    actor.y += 0x15;
    Sprite* sprite = &rocket_flames1;
    if(cur->y == 0xb8) {
        actorDestroy(cur, &rocket_flames1);
        actorDestroy(cur, &rocket_flames2);
        // to raf1
    } else if(cur->y < 0xb8) {
        if((getGameTime() & 0x4) == 0) {
            sprite = &rocket_flames2;
        }
        //RAF_0
        actorDestroy(cur, &rocket_flames1);
        actorDestroy(cur, &rocket_flames2);
        drawAnimSprite(cur, sprite);
        cur->color = 0x42;
        colorizeSprite(cur);
    }
    // RAF_1
    cur->y -= 0x15;
    actor.y -= 0x15;
}

void rocketModuleReset() {
    // zero from rocketModuleState to inactiveJetmanState
    #define FRM 5
    #define FTO 0xf
    for(int i = FRM; i <= FTO; ++i) {
        states[i]->spriteIndex = 0;
    }
}

void rocketTakeoff(State* cur){
    actorSaveSpritePos(cur);
    --cur->y;
    sfxThrusters();
    rocketAnimateFlames(cur);
    if(cur->y >= 0x28)
    {
        updateRocketColor(cur);
    } else {
        ++playerLevel;
        rocketModuleReset();
        ++(cur->spriteIndex);
        cur->fuelCollected = 0;
        levelNew();
    }
}
void rocketLanding(State* cur){
    actorSaveSpritePos(cur);
    ++cur->y;
    sfxThrusters();
    rocketAnimateFlames(cur);
    if(cur->y >= 0xb7)
    {   
        cur->spriteIndex = 0x9;
        playerInit();
    }
    updateRocketColor(cur);
}
void sfxEnemyDeath(State* cur){}
void sfxJetmanDeath(State* cur){}
void itemCheckCollect(State* cur){
    actorSaveSpritePos(cur);
    byte ret = checkPlatformCollision(cur);
    if(!(ret & 4))
        cur->y += 2;
    ret = collisionWithJetman(cur);
    if(!ret) {
        itemDropNew(cur);
    } else {
        Sprite* sprite = itemGetSpriteAddress(cur->jumpTableOffset);
        Coords c = getSpritePosition(sprite, actor.coords);
        SpriteData old = {
            .coords = c,
            .height = sprite->height,
            .width = sprite->width,
            .spritedata = sprite->data
        };
        maskSprite(&old, NULL);
        cur->spriteIndex = 0;
        addPointsToScore(250);
        sfxPickupItem();
    }
}
void ufoUpdate(State* cur){}
void laserBeamAnimate(State* cur){  // bit 0 is dir bit 2 is used or not
    LaserBeam* laser = (LaserBeam*) cur;
    if(laser->x[0] & 0x4) {
        sbyte a = ((laser->x[0] & 0x1) ? -8 : 8) + laser->x[0];
        Coords c = {.x = laser->x[0], .y = laser->y};
        if(c.y < 0x80 && getVideoByte(c) != 0) {
            // LaserBeamAnimate_5
            laser->x[0] &= ~0x4;
        } else {
            // LaserBeamAnimate_1
            laser->x[0] = a;
            byteOut(c, 0xff, EQUAL);
            setAttrib(c.x >> 3, c.y >> 3, (Attrib){.attrib = laser->color});
            laser->length -= 0x08;
            if((laser->length & 0xf8) == 0) {
                laser->x[0] &= ~0x4;
            }
        }
    } // else LaserBeamAnimate_3
    // LaserBeamAnimate_3
    byte b_1 = 0x03, c_1 = 0x1c, e_1 = 0xe0;
    for(byte p = 0; p < 3; ++p) {
        byte* pulse = &laser->x[p + 1];
        // LaserBeamAnimate_4
        byte a = ((*pulse) ^ (laser->x[0])) & 0xf8;
        if(a) {
            //LaserBeamAnimate_6
            a = *pulse;
            if(a & 4) {
                // LaserBeamAnimate_7
                sbyte offs = (a & 1 ? -8 : 8);
                *pulse += offs;
                Coords old = {.x = a, .y = laser->y};
                a = b_1;
                b_1 = c_1;
                c_1 = e_1;
                byteOut(old, a, AND);
            } else {
                if((--laser->length & 0x7) != 0)
                    break; // fast ret
                a = (byteRand() & 0x3) | 0x4;
                laser->length |= a;
                *pulse |= 4;
                break; // fast ret
            }
        } else {
            a = b_1; 
            b_1 = c_1;
            c_1 = e_1;
            if(p == 2) {
                laser->used = 0;
                break; //fast ret
            }
        }
    }
}
void squidgyAlienUpdate(State* cur){
    ++currentAlienNumber;
    actorSaveSpritePos(cur);
    actor.spriteIndex = cur->spriteIndex & 0xc0 | 0x3;
    if(laserBeamFire(cur) == 1) {
        // kill alien
        addPointsToScore(80);
        animationStateReset(cur);
        return sfxSetExplodeParam(cur, 0);
    }
    if(collisionWithJetman(cur))
        alienCollisionAnimSfx(cur);
    alienNewDirFlag = 0;
    while(alienNewDirFlag < 2) {
        byte e = checkPlatformCollision(cur);
        if(e) {
            printf("e is %x\n", e);
        }
        if(e & 0x84) {
            cur->moving.ud = 0;
        }
        else if(e & 0x14) {
            cur->moving.ud = 1;
        }
        else if(e & 0x4) {
            cur->umoving = (cur->umoving & 0xbf) | (e & 0x40);
            // fallthrough
        }
        // sau1 => ! e & 0x4
        cur->x += (cur->moving.rl ? +2 : -2);
        if(cur->moving.ud) {
            cur->y += 2;
        } else {
            cur->y -= 2;
            if(cur->y < 0x24) {
                cur->moving.ud = 1;
            }
        }
        ++alienNewDirFlag;
    }
    drawAlien(cur);

}

byte itemCalcDropColumn(void) {
    return itemDropPositionTable[rand() & 0xf];    
}

static inline bool invalidJetmanState() {
    return (
        jetmanState.spriteIndex == 0 ||
        (jetmanState.spriteIndex & 0x3f) >= 3
    );
}

void itemNewFuelPod() {
    if(
        invalidJetmanState() || // impossible?  
        rocketModuleState.type != RMUnused ||
        rocketState.fuelCollected >= 6 ||
        (~getGameTime() & 0xf) != 0
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
    if(currentState > maxState) {  
        while(isKeyDown(keyPause));
        byte r = byteRand();
#ifndef NDEBUG
        bool aliens = true;
#else
        bool aliens = true;
#endif
        if(
            aliens && (
                ( currentAlienNumber < 3) || 
                (!(r & 0x1f) && currentAlienNumber < 6)
             ) &&
            playerDelayCounter == 0 &&
            jetmanState.direction.fly == 0
        )
        {
            for(byte b = 0; b < array_sizeof(alienState); ++b) {
                if(alienState[b].type == RMUnused) {
                    alienState[b] = defaultAlienState;
                    r = (byteRand() & 1 ? 0x40 : 0); 
                    alienState[b].umoving = r; //moving
                    alienState[b].spriteIndex = r; //direction
                    r = (byteRand() & 0x7f) + 0x28;
                    alienState[b].y = r;
                    r = (byteRand() & 0x3) + 2;
                    alienState[b].color = r;
                    alienState[b].yspeed = (r & 1);
                    r = itemLevelObjectTypes[playerLevel & 0x07];
                    r |= (alienState[b].spriteIndex & 0xc0);
                    alienState[b].spriteIndex = r;
                    break;
                }
            }
        }
        itemNewFuelPod();
        itemNewCollectible();
        currentState = 0;
        currentAlienNumber = 0;
    }
}