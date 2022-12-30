#include <stdint.h>

typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint16_t word;

// utils
sbyte abs(sbyte b) {
    return(b < 0 ? -b : b);
}
sbyte sgn(sbyte b) {
    return (b > 0) - (0 > b);
}

// data types

extern byte hiScore[3];
typedef struct {
    byte players:1;
    byte input:1;
} GameOptions;
extern GameOptions gameOptions;

extern byte p1Score[3];
extern byte p2Score[3];

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
    byte unk:1;
    byte unused:4;
    byte rl:1; // 1 is left
    byte ud:1;  // 1 is down
}Moving;

#define MAXSPEED_WALKING 0x20
#define MAXSPEED_FLYING  0x40
#define MAXSPEED_VERTICAL 0x3f
#define JETMAN_HEIGHT 0x24

typedef struct {
    Direction direction;
    Coords coords;
    byte color;
    Moving moving;
    byte xspeed;
    byte yspeed;
    byte height;
} ActorState;

extern ActorState jetmanState;

enum LaserBeamUsed {
    LBUnused = 0,
    LBUsed = 0x10
};

typedef struct {
    LaserBeamUsed used;
    byte y;
    byte x[4];
    byte lenght;
    byte color;
} LaserBeam;

LaserBeam laserBeamParam[4];

typedef struct {
    byte frequency;
    byte duration;
} Sound;
extern Sound explosionSfxParams;

enum RMType {
    RMUnused = 0,
    RMRocket = 0x4,
    RMCollectible = 0xe
};
enum RMState {
    New = 1,
    Collected = 3,
    FreeFall = 5,
    Dropped = 7
};
typedef struct {
    RMType type;
    byte x;
    byte y;
    byte color;
    RMState state;
    byte unused;
    byte jumpTableOffset;
    byte height;
} ModuleState;

extern ModuleState rocketModuleState;
extern ModuleState itemState;

enum Animating {
    No = 0,
    Done = 3,
    Animating = 8
};
typedef struct {
    Animating animating;
    byte jetmanLastX;
    byte jetmanLastY;
    byte color;
    byte frame;
    byte unused1;
    byte unknown;
    byte unused2;
} AnimState;

extern AnimState jetmanThrusterAnimState;

// aliens
extern ActorState alienState[6];

extern AnimState jetmanExplodingAnimState;
extern ActorState inactiveJetmanState;
extern ActorState inactiveRocketState[3];

typedef struct {
    union {
        Coords coords;
        struct {
            byte x;
            byte y;
        };
    };
    Direction direction;
    byte height; //pix
    byte width;  //tiles
    byte spriteHeight;
    byte gfxDtaHeight;
    byte flyingMovement;
} ActorTempState;

extern ActorTempState actor;

extern byte alienNewDirFlag;

extern byte jetmanSpeedModifier;
extern byte currentAlienNumber;

extern word gameTime;

extern Coords actorCoords;
enum PlayerNum {
    Player1 = 0,
    Player2 = 0xff
};
extern PlayerNum currentPlayerNumber; // 0 is player 1, ff is player 2
extern byte jetmanRocketModConnected; // 1 at start/newlife, 0 if rocket attached
extern byte rocketModAttached; // 4 at start level, 2 if a rocket module attached
extern byte lastFrame;
extern byte frameTicked;
extern byte currentColorAttribute;

enum PlayerDelay {
    Player1Delay = 0x80,
    Player2Delay = 0xff
};
extern PlayerDelay playerDelayCounter; // 0x80 1 player, 0xff two player
extern byte playerLevel;
extern byte playerLives;
extern byte inactivePlayerLevel;
extern byte inactivePlayerLives;

typedef struct {
    byte header;
    byte width;
    byte height;
    byte pixel[0x31];
} Buffer;

extern Buffer bufferAliensRight[2];
extern Buffer bufferAliensLeft[2];
extern Buffer bufferItem[4];