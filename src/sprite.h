#pragma once
#include "data.h"


typedef struct {
    byte* spritedata;
    Coords coords;
    byte height;
    byte width; // should be constant
} SpriteData;

/**
 * Delete old sprite and draw next sprite, counting for differences
 * in height
*/
void updateAndEraseActor(State* state);


/**
 * return coords as (x + sprite.xoffs, y)
 * In the original code, HL returned the screen address. Seems meaningless now.
*/
Coords getSpritePosition(Sprite* sprite, Coords coords);

/**
 * very complex function in the asm version: the main register set contains what to mask
 * (c is height, hl is screen address, de is sprite data), alternative set contains what 
 * to draw (c' is height, h'l' is screen address, de is sprite data). If c or c' are zero,
 * the passage is skipped altogether.
*/
void maskSprite(SpriteData* old, SpriteData* newsprite);

/**
 * the function save in the actor buffer the position and spriteIndex of the sprite.
 * As a side note: actor should be renamed to "oldSpriteData". 
*/
void actorSaveSpritePos(State* cur);

/**
 * delete a Sprite for good, without replacing it with a new Sprite.
*/
Coords actorFindDestroy(State* state);

/**
 * Cancel and redraw a sprite in the new position.
*/
void actorUpdatePosition(State* state, Sprite* sprite);

/**
 * draw a collectible (or a rocket module)
*/
void getCollectibleID(State* state);

/**
 * Colour a sprite
*/
void colorizeSprite(State* state);

/**
 * Redraw a sprite
*/
void redrawSprite(State* cur);