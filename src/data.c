#include "data.h"

// all global/static variables are initialized to 0 (c std) 
uint32_t hiScore;
GameOptions gameOptions;
uint32_t p1Score;
uint32_t p2Score;
State jetmanState;
LaserBeam laserBeamParam[4];
SoundData explosionSfxParams;
State rocketState;
State rocketModuleState;
State itemState;
State jetmanThrusterAnimState;
State alienState[6];
State jetmanExplodingAnimState;
State inactiveJetmanState;
State inactiveRocketState[3];
ActorTempState actor;
byte jetmanSpeedModifier;
byte currentAlienNumber;
Coords actorCoords;
enum PlayerNum currentPlayerNumber;
byte jetmanRocketModConnected;
byte rocketModAttached;
byte lastFrame;
byte frameTicked;
byte currentColorAttribute;
byte playerDelayCounter;
byte playerLevel;
byte playerLives;
byte inactivePlayerLevel;
byte inactivePlayerLives;

State padding = {0, 0, 0, 0, 0, 0, 0, 0};
// state array
State* states[] = {
   (State*)&laserBeamParam[0],   //0
   (State*)&laserBeamParam[1],   //1
   (State*)&laserBeamParam[2],   //2
   (State*)&laserBeamParam[3],   //3
   &rocketState,                 //4
   &rocketModuleState,           //5
   &itemState,                   //6
   &jetmanThrusterAnimState,     //7
   &alienState[0],               //8
   &alienState[1],               //9
   &alienState[2],               //10 a
   &alienState[3],               //11 b
   &alienState[4],               //12 c
   &alienState[5],               //13 d
   &jetmanExplodingAnimState,    //14 e
   &inactiveJetmanState,         //15 f
   &padding,                     //16 10
   &inactiveRocketState[0],
   &inactiveRocketState[1],
   &inactiveRocketState[2],
   // there is some padding
   &padding,
   &padding
};
byte currentState;
byte maxState = 0x0f;

// inizialized data
GFXParams gfxParamsPlatforms[] = {
    {0x04, 0x80, 0x60, 0x1b},
    {0x06, 0x78, 0xb8, 0x88},
    {0x04, 0x30, 0x48, 0x23},
    {0x04, 0xd0, 0x30, 0x23}
};

State defaultPlayerState = {
    .spriteIndex = 0x01, 0x80, 0xb7, 0x47, .frame = 0x00, 0x00, 0x00, 0x24
};
State defaultRocketState[] = {
  {.spriteIndex = 0x09,0xa8,0xb7,0x02,.frame = 0x01,0x00,0x00,0x1c}, // Rocket mounted (base)
  {.spriteIndex = 0x04,0x30,0x47,0x47,.frame = 0x00,0x00,0x10,0x18}, // Top module state
  {.spriteIndex = 0x04,0x80,0x5f,0x47,.frame = 0x01,0x00,0x08,0x18} // Middle module state  
};
State defaultRocketModuleState = {
    .spriteIndex = 0x04, 0x00, 0x20, 0x43, .frame = 0x01, 0x00, 0x18, 0x18
};
State defaultItemState = {
    .spriteIndex = 0x0e, 0x00, 0x20, 0x00, .frame = 0x00, 0x00, 0x00, 0x18
};
State defaultAlienState = {
    .spriteIndex = 0x03, 0x00, 0x00, 0x42, .frame = 0x80, 0x04, 0x00, 0x1c
};

byte itemLevelObjectTypes[] = {
   0x03, 0x11, 0x06, 0x07, 0x0f, 0x05, 0x03, 0x0f
};

byte itemDropPositionTable[] = {
   0x08,0x20,0x28,0x30,0x38,0x40,0x58,0x60,
   0x78,0x80,0x88,0xc0,0xe0,0x08,0x58,0x60   
};

SoundData explosionSfxDefaults[] = {
   {0x0c, 0x04},
   {0x0d, 0x04}
};

byte laserBeamColors[] = {
   0x47, 0x43, 0x43, 0x45
};

Message menuCopyright = {
    .attrib = 0x47,
    .msg = "@1983 A.C.G. ALL RIGHTS RESERVED"
};

byte jetman_fly_right1_data[] = {
   0x10,0x00,0x20,0x00,0xd8,0x00,0x44,0x00 ,
   0x38,0x00,0x50,0x1e,0x00,0x1c,0x7c,0x00,
   0x54,0x18,0x29,0xf8,0x3d,0xf8,0x7b,0x80,
   0x5b,0xc0,0x74,0x3e,0x54,0xd0,0x74,0xd0,
   0x57,0x80,0x60,0x00,0x67,0xc0,0x2e,0xe0,
   0x2e,0xe0,0x2d,0xe0,0x0e,0x00,0x07,0x80
};
byte jetman_fly_right2_data[] = {
   0x0a,0x00,0x00,0x50,0x00,0x00,0x05,0x00,
   0x00,0x0a,0x00,0x00,0x08,0x80,0x00,0x15,
   0x07,0x80,0x00,0x07,0x00,0x1f,0x00,0x00,
   0x15,0x06,0x00,0x0a,0x7e,0x00,0x0f,0x7e,
   0x00,0x1e,0xe0,0x00,0x16,0xf0,0x00,0x1d,
   0x0f,0x80,0x15,0x34,0x00,0x1d,0x34,0x00,
   0x15,0xe0,0x00,0x18,0x00,0x00,0x19,0xf0,
   0x00,0x0b,0xb8,0x00,0x0b,0xb8,0x00,0x0b,
   0x78,0x00,0x03,0x80,0x00,0x01,0xe0,0x00
};

byte jetman_fly_right3_data[] = {
   0x01,0x00,0x00,0x04,0x00,0x00,0x10,0x80,
   0x00,0x06,0x00,0x00,0x00,0x80,0x00,0x06,
   0xc1,0xe0,0x00,0x01,0xc0,0x07,0xc0,0x00,
   0x05,0x41,0x80,0x02,0x9f,0x80,0x03,0xdf,
   0x80,0x07,0xb8,0x00,0x05,0xbc,0x00,0x07,
   0x43,0xe0,0x05,0x4d,0x00,0x07,0x4d,0x00,
   0x05,0x78,0x00,0x06,0x00,0x00,0x06,0x7c,
   0x00,0x02,0xee,0x00,0x02,0xee,0x00,0x02,
   0xde,0x00,0x00,0xe0,0x00,0x00,0x78,0x00
};

byte jetman_fly_right4_data[] = {
   0x08,0x00,0x00,0x00,0x80,0x00,0x09,0x20,
   0x00,0x02,0x40,0x00,0x00,0x90,0x00,0x02,
   0xd0,0x78,0x00,0x00,0x70,0x01,0xf0,0x00,
   0x01,0x50,0x60,0x00,0xa7,0xe0,0x00,0xf7,
   0xe0,0x01,0xee,0x00,0x01,0x6f,0x00,0x01,
   0xd0,0xf8,0x01,0x53,0x40,0x01,0xd3,0x40,
   0x01,0x5e,0x00,0x01,0x80,0x00,0x01,0x9f,
   0x00,0x00,0xbb,0x80,0x00,0xbb,0x80,0x00,
   0xb7,0x80,0x00,0x38,0x00,0x00,0x1e,0x00
};

byte jetman_fly_left1_data[] = {
   0x00,0x08,0x00,0x04,0x00,0x1b,0x00,0x22,
   0x00,0x1c,0x78,0x0a,0x38,0x00,0x00,0x3e,
   0x18,0x2a,0x1f,0x94,0x1f,0xbc,0x01,0xde,
   0x03,0xda,0x7c,0x2e,0x0b,0x2a,0x0b,0x2e,
   0x01,0xea,0x00,0x06,0x03,0xe6,0x07,0x74,
   0x07,0x74,0x07,0xb4,0x00,0x70,0x01,0xe0
};

byte jetman_fly_left2_data[] = {
   0x00,0x50,0x00,0x0a,0x00,0xa0,0x00,0x50,
   0x01,0x10,0xe0,0xa8,0xe0,0x00,0x00,0xf8,
   0x60,0xa8,0x7e,0x50,0x7e,0xf0,0x07,0x78,
   0x0f,0x68,0xf0,0xb8,0x2c,0xa8,0x2c,0xb8,
   0x07,0xa8,0x00,0x18,0x0f,0x98,0x1d,0xd0,
   0x1d,0xd0,0x1e,0xd0,0x01,0xc0,0x07,0x80
};

byte jetman_fly_left3_data[] = {
   0x00,0x00,0x80,0x00,0x00,0x20,0x00,0x01,
   0x08,0x00,0x00,0x60,0x00,0x01,0x00,0x07,
   0x83,0x60,0x03,0x80,0x00,0x00,0x03,0xe0,
   0x01,0x82,0xa0,0x01,0xf9,0x40,0x01,0xfb,
   0xc0,0x00,0x1d,0xe0,0x00,0x3d,0xa0,0x07,
   0xc2,0xe0,0x00,0xb2,0xa0,0x00,0xb2,0xe0,
   0x00,0x1e,0xa0,0x00,0x00,0x60,0x00,0x3e,
   0x60,0x00,0x77,0x40,0x00,0x77,0x40,0x00,
   0x7b,0x40,0x00,0x07,0x00,0x00,0x1e,0x00
};

byte jetman_fly_left4_data[] = {
   0x00,0x00,0x10,0x00,0x01,0x00,0x00,0x04,
   0x90,0x00,0x02,0x40,0x00,0x09,0x00,0x1e,
   0x0b,0x40,0x0e,0x00,0x00,0x00,0x0f,0x80,
   0x06,0x0a,0x80,0x07,0xe5,0x00,0x07,0xef,
   0x00,0x00,0x77,0x80,0x00,0xf6,0x80,0x1f,
   0x0b,0x80,0x02,0xca,0x80,0x02,0xcb,0x80,
   0x00,0x7a,0x80,0x00,0x01,0x80,0x00,0xf9,
   0x80,0x01,0xdd,0x00,0x01,0xdd,0x00,0x01,
   0xed,0x00,0x00,0x1c,0x00,0x00,0x78,0x00
};

byte jetman_walk_left1_data[] = {
   0x00,0x00,0x00,0x00,0x07,0x80,0x03,0x80,
   0x01,0x80,0x00,0x00,0x01,0x80,0x03,0xbe,
   0x03,0xaa,0x03,0xd4,0x03,0xfc,0x03,0xbe,
   0x03,0xda,0x7c,0x2e,0x0b,0x2a,0x0b,0x2e,
   0x01,0xea,0x00,0x06,0x03,0xe6,0x07,0x74,
   0x07,0x74,0x07,0xb4,0x00,0x70,0x01,0xe0  
};

byte jetman_walk_left2_data[] = {
   0x00,0x00,0x00,0x00,0x7f,0x80,0x3b,0x80,
   0x19,0x80,0x0b,0x00,0x17,0x00,0x0e,0xf8,
   0x0e,0xa8,0x0f,0x50,0x0f,0xf0,0x0f,0xe8,
   0x0f,0x68,0xf0,0xb8,0x2c,0xa8,0x2c,0xb8,
   0x07,0xa8,0x00,0x18,0x0f,0x98,0x1d,0xd0,
   0x1d,0xc8,0x1e,0xc8,0x01,0xc0,0x07,0x80
};

byte jetman_walk_left3_data[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x87,
   0x00,0x03,0x83,0x80,0x01,0x85,0x80,0x00,
   0xce,0x00,0x00,0xdc,0x00,0x00,0x7b,0xe0,
   0x00,0x7e,0xa0,0x00,0x3d,0x40,0x00,0x3f,
   0xc0,0x00,0x3d,0xe0,0x00,0x3d,0xa0,0x07,
   0xc2,0xe0,0x00,0xb2,0xa0,0x00,0xb2,0xe0,
   0x00,0x1e,0xa0,0x00,0x00,0x60,0x00,0x3e,
   0x60,0x00,0x77,0x40,0x00,0x77,0x40,0x00,
   0x7b,0x40,0x00,0x07,0x00,0x00,0x1e,0x00
};

byte jetman_walk_left4_data[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xf8,
   0x00,0x03,0xb8,0x00,0x01,0x98,0x00,0x00,
   0xb0,0x00,0x01,0x70,0x00,0x00,0xef,0x80,
   0x00,0xea,0x80,0x00,0xf5,0x00,0x00,0xff,
   0x00,0x00,0xf7,0x80,0x00,0xf6,0x80,0x1f,
   0x0b,0x80,0x02,0xca,0x80,0x02,0xcb,0x80,
   0x00,0x7a,0x80,0x00,0x01,0x80,0x00,0xf1,
   0x80,0x01,0xdc,0x80,0x01,0xdc,0x80,0x01,
   0xec,0x80,0x00,0x1c,0x00,0x00,0x78,0x00    
};

byte jetman_walk_right1_data[] = {
   0x00,0x00,0x00,0x00,0x01,0xe0,0x01,0xc0,
   0x01,0x80,0x00,0x00,0x01,0x80,0x7d,0xc0,
   0x55,0xc0,0x2b,0xc0,0x3f,0xc0,0x7b,0xc0,
   0x5b,0xc0,0x74,0x3e,0x54,0xd0,0x74,0xd0,
   0x57,0x80,0x60,0x00,0x67,0xc0,0x2e,0xe0,
   0x2e,0xe0,0x2d,0xe0,0x0e,0x00,0x07,0x80,
};

byte jetman_walk_right2_data[] = {
   0x00,0x00,0x00,0x00,0x01,0xfe,0x01,0xdc,
   0x01,0x98,0x00,0xd0,0x00,0xe8,0x1f,0x70,
   0x15,0x70,0x0a,0xf0,0x0f,0xf0,0x1e,0xf0,
   0x16,0xf0,0x1d,0x0f,0x15,0x34,0x1d,0x34,
   0x15,0xe0,0x18,0x00,0x19,0xf0,0x0b,0xb8,
   0x13,0xb8,0x13,0x78,0x03,0x80,0x01,0xe0
};

byte jetman_walk_right3_data[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe1,
   0xe0,0x01,0xc1,0xc0,0x01,0xa1,0x80,0x00,
   0x73,0x00,0x00,0x3b,0x00,0x07,0xde,0x00,
   0x05,0x7e,0x00,0x02,0xbc,0x00,0x03,0xfc,
   0x00,0x07,0xbc,0x00,0x05,0xbc,0x00,0x07,
   0x43,0xe0,0x05,0x4d,0x00,0x07,0x4d,0x00,
   0x05,0x78,0x00,0x06,0x00,0x00,0x06,0x7c,
   0x00,0x02,0xee,0x00,0x02,0xee,0x00,0x02,
   0xde,0x00,0x00,0xe0,0x00,0x00,0x78,0x00
};

byte jetman_walk_right4_data[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,
   0xe0,0x00,0x1d,0xc0,0x00,0x19,0x80,0x00,
   0x0d,0x00,0x00,0x0e,0x80,0x01,0xf7,0x00,
   0x01,0x57,0x00,0x00,0xaf,0x00,0x00,0xff,
   0x00,0x01,0xef,0x00,0x01,0x6f,0x00,0x01,
   0xd0,0xf8,0x01,0x53,0x40,0x01,0xd3,0x40,
   0x01,0x5e,0x00,0x01,0x80,0x00,0x01,0x8f,
   0x00,0x01,0x3b,0x80,0x01,0x3b,0x80,0x01,
   0x37,0x80,0x00,0x38,0x00,0x00,0x1e,0x00   
};

byte explosion_big_data[] = {
   0x01,0xf0,0x00,0x07,0xf8,0x86,0x0f,0xfe,
   0xf0,0x6b,0xfe,0xf8,0xfc,0xff,0xfc,0xff,
   0x7f,0x78,0xff,0xbe,0xe4,0xff,0x7e,0x5e,
   0x7e,0xf9,0xbf,0x7b,0xff,0xdf,0xdd,0xfe,
   0xff,0x3f,0xef,0xbe,0x6f,0xef,0xc4,0x67,
   0xd3,0xf8,0x3b,0x9c,0xe0,0x0f,0x0e,0xc0
};

byte explosion_medium_data[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7c,
   0x00,0x00,0xe6,0x00,0x0e,0xfe,0xc0,0x1f,
   0x7f,0xe0,0x1f,0xbf,0xc0,0x1f,0xd7,0xf8,
   0x0f,0xef,0xfc,0x1f,0xef,0xec,0x17,0xdf,
   0xbc,0x1e,0xff,0xd8,0x09,0xbf,0xc0,0x07,
   0x1f,0x80,0x00,0x00,0x00,0x00,0x00,0x00
};

byte explosion_small_data[] = {
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x0c,0x00,0x01,
   0xde,0xc0,0x03,0xdf,0xc0,0x03,0xef,0xc0,
   0x03,0xac,0x70,0x03,0xdf,0xf8,0x01,0xff,
   0xf8,0x00,0x2f,0xb0,0x00,0x33,0x00,0x00,
   0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

byte rocket_u3_bottom_data[] = {
   0x9e,0x4f,0x9e,0x4f,0x9e,0x4f,0x82,0x41,
   0x5c,0x2e,0x5c,0x2e,0x28,0x14,0x7f,0xfe,
   0x45,0xfe,0x45,0xfe,0x45,0xfe,0x45,0xfe,
   0x7f,0xfe,0xa6,0x65,0xa6,0x65,0xd9,0x98
};

byte rocket_u3_middle_data[] = {
   0xd9,0x9b,0xff,0xff,0x8b,0xff,0x8a,0x11,
   0x8a,0xdd,0x8a,0xd1,0x8a,0xdd,0x8a,0xd1,
   0x8b,0xff,0x8b,0xff,0xff,0xff,0xa6,0x65,
   0xa6,0x65,0xd9,0x9b,0xd9,0x9b,0x7f,0xfe
};

byte rocket_u3_top_data[] = {
   0x4b,0xfe,0x25,0xfc,0x25,0xfc,0x13,0xf8,
   0x1e,0x08,0x09,0xf0,0x09,0xf0,0x09,0xf0,
   0x08,0x10,0x05,0xe0,0x02,0x40,0x01,0x80,
   0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80
};

byte rocket_u1_bottom_data[] = {
   0xb8,0x17,0xb8,0x17,0xb9,0x97,0x89,0x91,
   0x52,0xca,0x55,0xea,0xf5,0xef,0xf4,0x2f,
   0xaf,0xf5,0xa9,0xf5,0xa9,0xf5,0xf9,0xff,
   0xb9,0xf7,0xb9,0xf7,0xb9,0xf7,0xb9,0xf7
};

byte rocket_u1_middle_data[] = {
   0xb9,0xf7,0xb9,0xf7,0xb9,0xf7,0xb9,0xf7,
   0x89,0xf1,0x59,0xf6,0x59,0xf6,0x29,0xf4,
   0x29,0xfc,0x19,0xf8,0x19,0x18,0x09,0xb0,
   0x09,0x30,0x09,0xb0,0x09,0xf0,0x09,0x10
};

byte rocket_u1_top_data[] = {
   0x09,0x50,0x09,0x50,0x09,0x50,0x09,0xf0,
   0x09,0xf0,0x09,0xf0,0x09,0xf0,0x09,0xf0,
   0x08,0x10,0x09,0xf0,0x05,0xe0,0x05,0xe0,
   0x02,0xc0,0x02,0xc0,0x01,0x80,0x01,0x80
};

byte rocket_u5_bottom_data[] = {
   0x80,0x4f,0x80,0x4f,0xc0,0x4f,0xdf,0x4f,
   0xd7,0x4f,0xd7,0x4f,0xd7,0x21,0xd7,0x2e,
   0xd7,0xa2,0x57,0x9c,0x57,0xfe,0x37,0x7e,
   0x37,0x76,0x1b,0x76,0x0b,0x76,0x0d,0x76
};
byte rocket_u5_middle_data[] = {
   0x0b,0x76,0x0c,0x7c,0x0f,0xf6,0x0f,0xf6,
   0x0b,0x16,0x0b,0x76,0x0b,0x16,0x0b,0xd6,
   0x0d,0x16,0x0b,0xf6,0x0b,0xf6,0x0b,0x16,
   0x0b,0x56,0x0d,0x52,0x0b,0x5a,0x0b,0x5a
};
byte rocket_u5_top_data[] = {
   0x0b,0xfa,0x0b,0xfa,0x0f,0xfa,0x0f,0xfa,
   0x0c,0xfa,0x0c,0xfa,0x07,0xf2,0x04,0xf2,
   0x04,0xf2,0x03,0xf2,0x01,0xf2,0x01,0xf2,
   0x00,0xf2,0x00,0xf2,0x00,0x7c,0x00,0x38
};

byte rocket_u4_bottom_data[] = {
   0x80,0x01,0x89,0xf1,0x89,0xf1,0xc9,0xf3,
   0xc8,0x13,0xe5,0xe7,0xe5,0xe7,0xf2,0x4f,
   0xf3,0xcf,0xfb,0xdd,0x8b,0xd1,0xab,0xd5,
   0xab,0xdf,0xab,0xd7,0xfb,0xdf,0xbb,0xdd
};
byte rocket_u4_middle_data[] = {
   0xbb,0xdd,0xbb,0xdd,0x97,0xe9,0x97,0xe9,
   0x97,0xe9,0x8f,0xf1,0x8f,0xf1,0x8f,0xf1,
   0x0f,0xf0,0x1f,0xf8,0x1f,0xf8,0x1f,0xf8,
   0x1d,0xb8,0x19,0x98,0x11,0x88,0x19,0x88
};
byte rocket_u4_top_data[] = {
   0x15,0x88,0x13,0x88,0x19,0x88,0x15,0x88,
   0x1b,0x98,0x09,0x90,0x09,0x90,0x0d,0xb0,
   0x05,0xa0,0x05,0xa0,0x07,0xe0,0x03,0xc0,
   0x03,0xc0,0x03,0xc0,0x01,0x80,0x01,0x80
};

byte gold_bar_data[] = {
   0xff,0xfc,0x80,0x0e,0x40,0x1e,0x40,0x1f,
   0x20,0x3f,0x3f,0xde,0x1f,0xec,0x0f,0xf8
};
byte fuel_pod_data[] = {
   0x18,0x18,0xff,0xff,0xff,0xff,0xb8,0x89,
   0xba,0xbb,0x8a,0x9b,0xba,0xbb,0x8a,0x8b,
   0xff,0xff,0xff,0xff,0x18,0x18
};
byte radiation_data[] = {
   0x0f,0xf8,0x10,0x04,0x23,0xe2,0x41,0xc1,
   0x60,0x83,0x20,0x82,0x6f,0x7b,0x4e,0x39,
   0x24,0x12,0x10,0x04,0x0f,0xf8
};
byte chemical_weapon_data[] = {
   0x70,0x1c,0xf8,0x3e,0xbf,0xee,0x98,0x26,
   0x70,0x1c,0x20,0x08,0x10,0x10,0x10,0x10,
   0x0b,0xa0,0x0f,0xe0,0x05,0xc0,0x04,0xc0,
   0x03,0x80
};
byte plutonium_data[] = {
   0x3f,0xfc,0x7f,0xfe,0xff,0xff,0xcf,0xff,
   0xc7,0xff,0xc3,0xff,0x60,0x7e,0x38,0x3c,
   0x0f,0xf0
};
byte diamond_data[] = {
   0x01,0x80,0x03,0xc0,0x07,0xe0,0x0f,0xf0,
   0x1f,0xf8,0x38,0x1c,0x68,0x16,0x47,0xe2,
   0x2f,0xf4,0x1f,0xf8,0x0f,0xf0,0x03,0xc0
};

byte rocket_flames1_data[] = {
   0x00, 0x40, 0x04, 0x88, 0x02, 0x40, 0x10, 0x90,
     0x0b, 0x58, 0x24, 0x62, 0x53, 0xb4, 0x37, 0x6a,
     0x8a, 0x5a, 0x57, 0xed, 0x2e, 0xf4, 0xb7, 0xfd,
     0x5e, 0xec, 0x7f, 0xf4, 0x2f, 0xfc, 0x00, 0x00
};
byte rocket_flames2_data[] = {
  	 0x00,0x80,0x02,0x00,0x00,0x00,0x00,0x20,
  	 0x12,0x40,0x00,0x00,0x09,0xa4,0x12,0xc8,
  	 0x4e,0x18,0x15,0xd2,0x2a,0xa9,0x5b,0xd4,
  	 0x2d,0x2e,0x12,0xf4,0xbf,0x7a,0x1f,0xfc
};

#define SPRITE(n, a, b, c) Sprite n = {.xoffset = a, .width = b, .height = c, .data = n##_data}

SPRITE(jetman_fly_right1, 0, 2, 0x18);
SPRITE(jetman_fly_right2, 0, 3, 0x18);
SPRITE(jetman_fly_right3, 0, 3, 0x18);
SPRITE(jetman_fly_right4, 0, 3, 0x18);
SPRITE(jetman_fly_left1, 8, 2, 0x18);
SPRITE(jetman_fly_left2, 8, 2, 0x18);
SPRITE(jetman_fly_left3, 0, 3, 0x18);
SPRITE(jetman_fly_left4, 0, 3, 0x18);
SPRITE(jetman_walk_left1, 8, 2, 0x18);
SPRITE(jetman_walk_left2, 8, 2, 0x18);
SPRITE(jetman_walk_left3, 0, 3, 0x18);
SPRITE(jetman_walk_left4, 0, 3, 0x18);
SPRITE(jetman_walk_right1, 0, 2, 0x18);
SPRITE(jetman_walk_right2, 0, 2, 0x18);
SPRITE(jetman_walk_right3, 0, 3, 0x18);
SPRITE(jetman_walk_right4, 0, 3, 0x18);
SPRITE(explosion_big, 0, 3, 0x10);
SPRITE(explosion_medium, 0, 3, 0x10);
SPRITE(explosion_small, 0, 3, 0x10);

Sprite* explosionSpriteTable[] = {
    &explosion_small,
    &explosion_medium,
    &explosion_big,
    &explosion_small,
    &explosion_medium,
    &explosion_big
};


SPRITE(rocket_u3_bottom, 0, 2, 0x10);
SPRITE(rocket_u3_middle, 0, 2, 0x10);
SPRITE(rocket_u3_top,    0, 2, 0x10);
SPRITE(rocket_u1_bottom, 0, 2, 0x10);
SPRITE(rocket_u1_middle, 0, 2, 0x10);
SPRITE(rocket_u1_top,    0, 2, 0x10);
SPRITE(rocket_u5_bottom, 0, 2, 0x10);
SPRITE(rocket_u5_middle, 0, 2, 0x10);
SPRITE(rocket_u5_top,    0, 2, 0x10);
SPRITE(rocket_u4_bottom, 0, 2, 0x10);
SPRITE(rocket_u4_middle, 0, 2, 0x10);
SPRITE(rocket_u4_top,    0, 2, 0x10);
SPRITE(gold_bar, 0, 2, 0x8);
SPRITE(fuel_pod, 0, 2, 0xb);
SPRITE(radiation, 0, 2, 0xb);
SPRITE(chemical_weapon, 0, 2, 0xd);
SPRITE(plutonium, 0, 2, 0x9);
SPRITE(diamond, 0, 2, 0xc);
SPRITE(rocket_flames1, 0, 2, 0xf);
SPRITE(rocket_flames2, 0, 2, 0xf);

Sprite* collectibleSpriteTable[] = {
  &rocket_u1_bottom ,  // Offset: $00 - Rocket #1: Bottom
  &rocket_u5_bottom ,  // Offset: $02 - Rocket #5: Bottom
  &rocket_u3_bottom ,  // Offset: $04 - Rocket #3: Bottom
  &rocket_u4_bottom ,  // Offset: $06 - Rocket #4: Bottom
  &rocket_u1_middle ,  // Offset: $08 - Rocket #1: Middle
  &rocket_u5_middle ,  // Offset: $0A - Rocket #5: Middle
  &rocket_u3_middle ,  // Offset: $0C - Rocket #3: Middle
  &rocket_u4_middle ,  // Offset: $0E - Rocket #4: Middle
  &rocket_u1_top  ,  // Offset: $10 - Rocket #1: Top
  &rocket_u5_top  ,  // Offset: $12 - Rocket #5: Top
  &rocket_u3_top  ,  // Offset: $14 - Rocket #3: Top
  &rocket_u4_top  ,  // Offset: $16 - Rocket #4: Top
  &fuel_pod       ,  // Offset: $18 - Fuel Cell
  &fuel_pod       ,  // Offset: $1A - Fuel Cell
  &fuel_pod       ,  // Offset: $1C - Fuel Cell
  &fuel_pod       ,  // Offset: $1E - Fuel Cell
  &gold_bar       ,  // Offset: $20 - Gold Bar
  &radiation      ,  // Offset: $22 - Radiation
  &chemical_weapon ,  // Offset: $24 - Chemical Weapon
  &plutonium      ,  // Offset: $26 - Plutonium
  &diamond          // Offset: $28 - Diamond    
};

byte meteor1_data[] = {
   0x02,0xf8,0x02,0xec,0x51,0x8e,0x27,0xe3,
   0x93,0xf9,0xef,0xe3,0x27,0x9d,0x5b,0xc2,
   0x25,0xf4,0x01,0x78,0x08,0x10
};

byte meteor2_data[] = {
   0x04,0x78,0x02,0x8c,0x25,0xa6,0x17,0xdf,
   0xc3,0xe3,0x5b,0x8f,0x16,0x3f,0x4d,0x9a,
   0x01,0xe4,0x04,0x70,0x00,0x90    
};

byte squidgy_alien1_data[] = {
   0x12,0x40,0x1a,0x94,0x6b,0xea,0x5f,0xff, 
   0x3f,0xfe,0xf9,0x9f,0x36,0x6c,0xf6,0x6e,
   0x79,0x9f,0x3f,0xfe,0x4f,0xfd,0x0f,0xf4,
   0x16,0xe8,0x0a,0x44
};
byte squidgy_alien2_data[] = {
   0x0a,0x44,0x16,0xe8,0x0f,0xf4,0x4d,0xfd,
   0x3f,0xfe,0x79,0x9f,0xf6,0x6e,0x36,0x6c,
   0xf9,0x9f,0x3f,0xfe,0x5f,0xff,0x6b,0xea,
   0x1a,0x94,0x12,0x40
};
byte jet_fighter_data[] = {
   0x1f,0xf0,0xc7,0x80,0xfb,0xdf,0xff,0xec,
   0xcf,0xf0,0x1e,0x00,0x78,0x00
};
byte ufo_data[] = {
   0x3f,0xfc,0x7f,0xfe,0xd9,0x9b,0x7f,0xfe,
   0x3f,0xfc,0x0d,0xb0,0x07,0xe0,0x01,0x80
};
byte sphere_alien1_data[] = {
   0x07,0xe0,0x1f,0xf8,0x3f,0xfc,0x7f,0xfe,
   0x7f,0xfe,0xff,0xff,0xff,0xff,0xff,0xff,
   0xe7,0xff,0xe7,0xff,0xe3,0xff,0x71,0xfe,
   0x79,0xfe,0x3f,0xfc,0x1f,0xf8,0x07,0xe0
};
byte sphere_alien2_data[] = {
   0x07,0xe0,0x1f,0xf8,0x3f,0xfc,0x7f,0xfe,
   0x7f,0xfe,0xff,0xff,0xff,0xff,0xe7,0xff,
   0xe7,0xff,0x63,0xfe,0x71,0xfe,0x39,0xfc,
   0x1f,0xf8,0x07,0xe0
};
byte cross_ship_data[] = {
   0x03,0x80,0x05,0xc0,0x04,0x40,0x07,0xc0,
   0x04,0x40,0x7b,0xbc,0xf4,0x5e,0xb5,0x56,
   0x94,0x52,0x7b,0xbc,0x04,0x40,0x07,0xc0,
   0x05,0xc0,0x04,0xc0,0x03,0x80
};
byte space_craft_data[] = {
   0x10,0x00,0x1e,0x00,0x3f,0xc0,0x61,0xf8,
   0xc0,0x3f,0xff,0xff,0xca,0xc0,0xca,0xc0,
   0xff,0xff,0xc0,0x3f,0x61,0xf8,0x3f,0xc0,
   0x1e,0x00,0x10,0x00
};
byte frog_alien_data[] = {
   0x0c,0x30,0x13,0xc8,0x3f,0xfc,0x7f,0xfe,
   0xff,0xff,0xdf,0xff,0xdf,0xff,0x48,0x9e,
   0x46,0x6e,0x2f,0xf4,0x19,0x98,0x19,0x98,
   0x0f,0xf0,0x06,0x60
};

#define ALIENSPRITE(n, a) AlienSprite n = {.height = a, .data = n##_data}

ALIENSPRITE(meteor1, 0x0b);
ALIENSPRITE(meteor2, 0x0b);
ALIENSPRITE(squidgy_alien1, 0x0e);
ALIENSPRITE(squidgy_alien2, 0x0e);
ALIENSPRITE(jet_fighter, 0x7);
ALIENSPRITE(ufo, 0x08);
ALIENSPRITE(sphere_alien1, 0x10);
ALIENSPRITE(sphere_alien2, 0x0e);
ALIENSPRITE(cross_ship, 0x0f);
ALIENSPRITE(space_craft, 0x0e);
ALIENSPRITE(frog_alien, 0x0e);

byte buffers[8][0x30];
Sprite bufferAliensRight[2] = {
   {0, 0, 0, buffers[0]},
   {0, 0, 0, buffers[1]}
};
Sprite bufferAliensLeft[2] = {
   {0, 0, 0, buffers[2]},
   {0, 0, 0, buffers[3]}
};
Sprite bufferItems[4] = {
   {0, 0, 0, buffers[4]},
   {0, 0, 0, buffers[5]},
   {0, 0, 0, buffers[6]},
   {0, 0, 0, buffers[7]}
};

Sprite* spriteTable[] = {
    &jetman_fly_right1,       // 0
    &jetman_fly_right2,
    &jetman_fly_right3,
    &jetman_fly_right4,
    &jetman_fly_left4,        // 4  
    &jetman_fly_left3,
    &jetman_fly_left2,
    &jetman_fly_left1,
    &jetman_walk_right1,      //8
    &jetman_walk_right2,      
    &jetman_walk_right3,
    &jetman_walk_right4,
    &jetman_walk_left4,       //0c 12
    &jetman_walk_left3,
    &jetman_walk_left2,       
    &jetman_walk_left1,        
    &bufferAliensRight[0],    //10 16
    &bufferAliensRight[0],
    &bufferAliensRight[1],
    &bufferAliensRight[1],
    &bufferAliensLeft[1],     //14 20
    &bufferAliensLeft[1],
    &bufferAliensLeft[0],
    &bufferAliensLeft[0],
    &bufferItems[0],          //18 24
    &bufferItems[1],
    &bufferItems[2],
    &bufferItems[3],
    &bufferItems[3],          //1c 28
    &bufferItems[2],
    &bufferItems[1],
    &bufferItems[0]           //1f 31
};
AlienSprite* alienSpriteTable[] = {
    &meteor1,
    &meteor2,
    &squidgy_alien1,
    &squidgy_alien2,
    &sphere_alien1,
    &sphere_alien2,
    &jet_fighter,
    &jet_fighter,
    &ufo,
    &ufo,
    &cross_ship,
    &cross_ship,
    &space_craft,
    &space_craft,
    &frog_alien,
    &frog_alien
};

byte tileLifeIcon[] = {
    0x18,0x24,0x3c,0x7e,0x5a,0x3c,0x3c,0x66
};

byte tilePlatformLeft[] = {
   0x2f,0x7f,0xff,0xdd,0xfb,0x7b,0x71,0x21
};
byte tilePlatformMiddle[] ={
   0xbd,0xff,0xff,0xf7,0xeb,0xdd,0xad,0x04
};
byte tilePlatformRight[] ={
   0x4c,0xfe,0xff,0x3e,0xff,0xfe,0x9c,0x08  
};