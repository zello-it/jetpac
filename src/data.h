#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint16_t word;
#define array_sizeof(n) (sizeof(n)/sizeof(n[0]))
#define LOBYTE(n) ((byte)(n & 0xff))
#define HIBYTE(n) ((byte)((n & 0xff00) >> 8))

// utils
static inline sbyte byteAbs(sbyte b) {
    return(b < 0 ? -b : b);
}
static inline sbyte byteSgn(sbyte b) {
    return (b > 0) - (0 > b);
}

static inline void swap(byte* one, byte* two) {
    byte tmp = *one;
    *one = *two;
    *two = tmp;
}

static inline byte byteRand() {
    return (byte)(rand());
}

static inline char* numToChar(int num) {
    char* ret = NULL;
    size_t sz = 0;
    sz = snprintf(ret, sz, "%d", num) + 1;
    ret = malloc(sz);
    snprintf(ret, sz, "%d", num);
    return ret;
}

static inline byte umin(byte one, byte two) {
    return (one < two ? one : two);
}

// data types

extern uint32_t hiScore;
typedef struct {
    byte players:1;
    byte input:1;
} GameOptions;
extern GameOptions gameOptions;

extern uint32_t p1Score;
extern uint32_t p2Score;

typedef struct {
    byte fly:1; 
    byte walk:1;    // this is useless...
    byte unused:4;
    byte left:1;
    byte down:1;
} Direction;

typedef struct{
    byte x;
    byte y;
}Coords;

#define JETMAN_X_START 0x80
#define JETMAN_Y_START 0xb7

typedef struct{
    byte hv:1; // 1 is vertical
    byte jet_moving:1;
    byte hit:1;
    byte unk:1;
    byte rebound:1;
    byte unused:1;
    byte rl:1; // 1 is left
    byte ud:1;  // 1 is down
}Moving;

#define MAXSPEED_WALKING 0x20
#define MAXSPEED_FLYING  0x40
#define MAXSPEED_VERTICAL 0x3f
#define JETMAN_HEIGHT 0x24

enum RMType {
    RMUnused = 0,
    RMRocket = 0x4,
    RMCollectible = 0xe
};
enum RMState {
    Free = 1,
    Carrying = 2,
    Drop = 4,
 //   FreeFall = 5,
 //   Dropped = 7
};

enum Animating {
    No = 0,
    Done = 3,
    Animating = 8
};


typedef struct {
    union {
        enum Animating animating;
        Direction direction;
        enum RMType type;
        byte spriteIndex;
    };
    byte x;
    byte y;
    byte color;
    union {
        Moving moving;
        enum RMState state;
        byte frame;
        byte umoving;
    };
    union {
        byte xspeed;
        byte fuelCollected;
        byte animByte;
    };
    union {
        byte yspeed;
        byte jumpTableOffset;
        byte oldSpriteIndex;
    };
    byte height;
} State;
extern State jetmanState;

enum LaserBeamUsed {
    LBUnused = 0,
    LBUsed = 0x10
};

typedef struct {
    enum LaserBeamUsed used;
    byte y;
    byte x[4];
    byte length;
    byte color;
} LaserBeam;

extern LaserBeam laserBeamParam[4];

typedef struct {
    byte frequency;
    byte duration;
} SoundData;
extern SoundData explosionSfxParams;
extern SoundData explosionSfxDefaults[];

extern State rocketState;
extern State rocketModuleState;
extern State itemState;

extern State jetmanThrusterAnimState;

// aliens
extern State alienState[6];

extern State jetmanExplodingAnimState;
extern State inactiveJetmanState;
extern State inactiveRocketState[3];

extern byte currentState;
extern byte maxState;
extern State* states[];

typedef struct {
    union {
        Coords coords;
        struct {
            byte x;
            byte y;
        };
    };
    byte spriteIndex;
    byte height; //pix
    byte width;  //tiles
    byte spriteHeight;
    byte gfxDtaHeight;
    byte xSpeedDecimal;
    byte ySpeedDecimal;
} ActorTempState;

extern ActorTempState actor;

extern byte jetmanSpeedModifier;
extern byte currentAlienNumber;


extern Coords actorCoords;
enum PlayerNum {
    Player1 = 0,
    Player2 = 0xff
};
extern enum PlayerNum currentPlayerNumber; // 0 is player 1, ff is player 2
extern byte jetmanRocketModConnected; // 1 at start/newlife, 0 if rocket attached
extern byte rocketModAttached; // 4 at start level, 2 if a rocket module attached
extern byte lastFrame;
extern byte frameTicked;
extern byte currentColorAttribute;

enum PlayerDelay {
    Player1Delay = 0x80,
    Player2Delay = 0xff
};
extern byte playerDelayCounter; // 0x80 1 player, 0xff two player
extern byte playerLevel;
extern byte playerLives;
extern byte inactivePlayerLevel;
extern byte inactivePlayerLives;


typedef struct {
    byte xoffset;
    byte width;
    byte height;
    byte* data;
} Sprite;

extern Sprite bufferAliensRight[2];
extern Sprite bufferAliensLeft[2];
extern Sprite bufferItems[4];
extern byte buffers[8][0x30];

typedef struct {
    byte color;
    union {
        Coords coords;
        struct {
            byte x; // right x coord / 2 - tile
            byte y;
        };
    };
    byte width;  // (width + tile) / 2
} GFXParams;
extern GFXParams gfxParamsPlatforms[4];
extern State defaultPlayerState;
extern State defaultRocketState[3];
extern State defaultRocketModuleState;
extern State defaultItemState;
extern State defaultAlienState;

typedef struct {
    byte attrib;
    const char* msg;
} Message;

extern Message menuCopyright;


typedef struct {
    byte height;
    byte* data;
} AlienSprite;

extern byte tileLifeIcon[];
extern byte tilePlatformLeft[];
extern byte tilePlatformMiddle[];
extern byte tilePlatformRight[];

extern Sprite* collectibleSpriteTable[];
extern Sprite* spriteTable[];
extern AlienSprite* alienSpriteTable[];
extern Sprite* explosionSpriteTable[];
extern Sprite rocket_flames1;
extern Sprite rocket_flames2;
extern byte itemLevelObjectTypes[8];
extern byte itemDropPositionTable[];
extern byte laserBeamColors[];
