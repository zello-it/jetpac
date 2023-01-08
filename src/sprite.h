#pragma once
#include "data.h"


typedef struct {
    byte* spritedata;
    Coords coords;
    byte height;
    byte width; // should be constant
} SpriteData;

void updateAndEraseActor(State* state);
Coords actorUpdate(State* state, Sprite* sprite);
Coords getSpritePosition(Sprite* sprite, Coords coords);
void maskSprite(SpriteData* old, SpriteData* newsprite);
Sprite* getSpriteAddress(byte x, byte header);
void actorSaveSpritePos(State* cur);
Coords actorFindDestroy(State* state);
void actorUpdatePosition(State* state, Sprite* sprite);