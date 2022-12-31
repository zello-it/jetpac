#include "data.h"

// all global/static variables are initialized to 0 (c std) 
byte hiScore[3];
GameOptions gameOptions;
byte p1Score[3];
byte p2Score[3];
ActorState jetmanState;
LaserBeam laserBeamParam[4];
SoundData explosionSfxParams;
ModuleState rocketModuleState;
ModuleState itemState;
AnimState jetmanThrusterAnimState;
ActorState alienState[6];
AnimState jetmanExplodingAnimState;
ActorState inactiveJetmanState;
ActorState inactiveRocketState[3];
ActorTempState actor;
byte alienNewDirFlag;
byte jetmanSpeedModifier;
byte currentAlienNumber;
word gameTime;
Coords actorCoords;
enum PlayerNum currentPlayerNumber;
byte jetmanRocketModConnected;
byte rocketModAttached;
byte lastFrame;
byte frameTicked;
byte currentColorAttribute;
enum PlayerDelay playerDelayCounter;
byte playerLevel;
byte playerLives;
byte inactivePlayerLevel;
byte inactivePlayerLives;
Buffer bufferAliensRight[2];
Buffer bufferAliensLeft[2];
Buffer bufferItem[4];

// inizialized data
GFXParams gfxParamsPlatforms[] = {
    {0x04, 0x80, 0x60, 0x1b},
    {0x06, 0x78, 0xb8, 0x88},
    {0x04, 0x30, 0x48, 0x23},
    {0x04, 0xd0, 0x30, 0x23}
};

ActorState defaultPlayerState = {
    .udirection = 0x01, 0x80, 0xb7, 0x47, .umoving = 0x00, 0x00, 0x00, 0x24
};
ActorState defaultRocketState[] = {
  {.udirection = 0x09,0xa8,0xb7,0x02,.umoving = 0x01,0x00,0x00,0x1c}, // Rocket state
  {.udirection = 0x04,0x30,0x47,0x47,.umoving = 0x00,0x00,0x10,0x18}, // Top module state
  {.udirection = 0x04,0x80,0x5f,0x47,.umoving = 0x01,0x00,0x08,0x18} // Middle module state  
};
ActorState defaultRocketModuleState = {
    .udirection = 0x04, 0x00, 0x20, 0x43, .umoving = 0x01, 0x00, 0x18, 0x18
};
ActorState defaultCollectibleItemState = {
    .udirection = 0x0e, 0x00, 0x20, 0x00, .umoving = 0x00, 0x00, 0x00, 0x18
};

Message menuCopyright = {
    .attrib = 0x47,
    .msg = "@ 1983 A.C.G. ALL RIGHTS RESERVED"
};