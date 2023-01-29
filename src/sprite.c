#include "sprite.h"
#include "video.h"
#include <stdbool.h>
#include <assert.h>


static void actorEraseMovedSprite(SpriteData* oldSprite, SpriteData* newSprite) {
    // should be the same: the y coords are not adjusted: there is no need to pass 
    // state as argument
    sbyte diff = oldSprite->coords.y - newSprite->coords.y;
    //sbyte diff = actor.y - cur->y;
    switch(byteSgn(diff)) {
    case 0:
        oldSprite->height = actor.spriteHeight;
        newSprite->height = actor.gfxDtaHeight;
        actor.spriteHeight = actor.gfxDtaHeight = 0;
        maskSprite(oldSprite, newSprite);
        break;
    case -1:
        diff = byteAbs(diff);
        if(actor.gfxDtaHeight < diff) {
            oldSprite->height = actor.spriteHeight;
            newSprite->height = actor.gfxDtaHeight;
            actor.spriteHeight = actor.gfxDtaHeight = 0;
            // register are swapped!!
            maskSprite(newSprite, oldSprite);
        } else {
            newSprite->height = actor.gfxDtaHeight;
            actor.gfxDtaHeight = actor.gfxDtaHeight - diff;
            maskSprite(oldSprite, newSprite);
        }
        break;

    case 1: 
        if(actor.spriteHeight < diff) {
            newSprite->height = actor.gfxDtaHeight;
            oldSprite->height = actor.spriteHeight;
            actor.spriteHeight = actor.gfxDtaHeight = 0;    
            maskSprite(oldSprite, newSprite);
        } else {
            oldSprite->height = actor.spriteHeight;
            actor.spriteHeight -= diff;
            maskSprite(oldSprite, newSprite);
        }
         break;
    } // end switch
}


static void actorEraseDestroyed(State* state, Sprite* sprite, Coords coords) {
    byte c = actor.spriteHeight;
    actor.gfxDtaHeight = 0;
    actor.height = 0;
    SpriteData sd = {
        .spritedata = sprite->data,
        .coords = coords,
        .height = c,
        .width = sprite->width
    };
    maskSprite(&sd, NULL);
}

static void writeSprite(SpriteData* sd, enum Operator op) {
    byte* ptr = sd->spritedata;
    while(sd->height--) {
        for(byte col = 0; col < sd->width; ++col) {
            byte out = *ptr++;
            if(op == AND)
                out = ~out;
            byteOutNoLock(
                (Coords){.x = sd->coords.x + 8 * col, .y = sd->coords.y},
                out,
                op
            );
        }
        --sd->coords.y;
        if(sd->coords.y > 191)
            sd->coords.y = 191;
    }
}


static Coords actorUpdate(State* state, Sprite* sprite) {
    actorCoords = (Coords) {.x = state->x + sprite->xoffset, .y = state->y};
    actor.width = sprite->width;
    actor.gfxDtaHeight = actor.height = sprite->height;
    return actorCoords;
}


Sprite* getSpriteAddress(byte x, byte header) {
    if(header & 0x40) {  // Considera la direzione 
        x |= 0x8;        
    }
    --header;
    byte index = (header << 4) | x; // il mask (& 0xf0) non serve, non stiamo ruotando
    index >>= 1; // word to index
//    printf("Asking for %x, returned %d\n", header, index);
    return spriteTable[index];
}

void updateAndEraseActor(State* state) {
    /*
    * In original code: FindActorSpriteAndUpdate, which is
    * ActorMoveSprite => getSpriteAddress + ActorUpdate
    * ActorUpdate return hl (sprite coords + offsetx), a (sprite height), de (sprite data)
    * and save in actor.gfxDtaHeight & gfxData the height, in actor.width the width.
    * Then it switch register set (exx) and calls JumpActorFindPosDir, which:
    * - finds the address of the sprite pointed by actor (old Sprite)
    * - find its position using getSpritePosition (de to sprite data, a is height, b is width)
    * Finally it drops in ActorEraseMovedSprite
    * Note: getSpritePosition is essential in "correcting" the sprite coords in case of 
    * Left movements.
    */
    Sprite* s = getSpriteAddress(state->x & 0x6, state->spriteIndex);
    actorUpdate(state, s);
    SpriteData newSprite = {
        .spritedata = s->data,
        .height = s->height,
        .coords = actorCoords,
        .width = s->width
    };
    Sprite* spriteActor = getSpriteAddress(actor.x & 0x6, actor.spriteIndex);
    Coords oldcoords = getSpritePosition(spriteActor, actor.coords);
    SpriteData oldSprite = {
        .spritedata = spriteActor->data,
        .height = spriteActor->height,
        .coords = oldcoords,
        .width = spriteActor->width
    };
    actorEraseMovedSprite(&oldSprite, &newSprite);
}

Coords getSpritePosition(Sprite* sprite, Coords coords) {
    Coords ret;
    ret.x = sprite->xoffset + coords.x;
    ret.y = coords.y;
    actor.spriteHeight = sprite->height;
    return ret;
}

void maskSprite(SpriteData* maskSprite, SpriteData* newSprite) {
    // if(maskSprite && newSprite && (maskSprite->coords.x != newSprite->coords.x)) {
    //     printf("Deleting %x @%d, %d h = %d\n", (uint) maskSprite->spritedata, maskSprite->coords.x, maskSprite->coords.y, maskSprite->height);
    //     printf("Writing %x @%d, %d h = %d\n", (uint) newSprite->spritedata, newSprite->coords.x, newSprite->coords.y, newSprite->height);
    //     printf("******\n");
    // }
    lockVideo();
    if(maskSprite && maskSprite->height) {  
        writeSprite(maskSprite, AND);
    }
    if(newSprite && newSprite->height) {
        writeSprite(newSprite, OR);
    }
    unlockVideo();
}


/**
 * Update actor tmp buffer with position and sprite index of the 
 * current object
*/
void actorSaveSpritePos(State* cur) {    //actorUpdatePosDir
    actor.x = cur->x;
    actor.y = cur->y;
    actor.spriteIndex = cur->spriteIndex;
}


Coords actorFindDestroy(State* state){
    Sprite* sprite = getSpriteAddress(actor.x & 0x6, actor.spriteIndex);
    Coords coords = getSpritePosition(sprite, actor.coords);
    actorEraseDestroyed(state, sprite, coords);
    return coords;
}


void actorUpdatePosition(State* state, Sprite* sprite) {
    Coords oldcoords = getSpritePosition(sprite, actor.coords);
    actorUpdate(state, sprite);
    SpriteData oldSprite = {
        .spritedata = sprite->data,
        .height = actor.height,
        .width = actor.width,
        .coords = oldcoords
    };
    SpriteData newSprite = {
        .spritedata = sprite->data,
        .height = sprite->height,
        .width = sprite->width,
        .coords = (Coords){state->x, state->y}//actorCoords
    };
    actorEraseMovedSprite(&oldSprite, &newSprite);
}

void getCollectibleID(State* state) {
    byte idx = (playerLevel / 2) & 0x6;
    idx += state->oldSpriteIndex;
    Sprite* sprite = collectibleSpriteTable[idx / 2];
    actorUpdate(state, sprite);
    actor.spriteHeight = 0;
    SpriteData sd = {
        .spritedata = sprite->data,
        .height = sprite->height,
        .coords = actorCoords,
        .width = sprite->width
    };
    maskSprite(NULL, &sd);
    colorizeSprite(state);
}


void colorizeSprite(State* state) {
   Attrib a = {.attrib = state->color};
   for(byte h = 0; h <= (actor.height + 4) / 8; ++h) {
        byte row = (byte)(actorCoords.y / 8 - h);
        if(row > 23) continue;
        for(byte w = 0; w < actor.width; ++w ) {
            byte col = (actorCoords.x / 8 + w) % 32;
            setAttrib(col, row, a);
        }
   } 
}

void redrawSprite(State* cur) {
    updateAndEraseActor(cur);
    colorizeSprite(cur);
}

void itemDrawSprite(State* cur) {
    Sprite* sprite = itemGetSpriteAddress(cur->jumpTableOffset);
    actorUpdatePosition(cur, sprite);
    colorizeSprite(cur);
}

Sprite* itemGetSpriteAddress(byte offset) {
    return(collectibleSpriteTable[offset / 2]);
}

void actorDestroy(State* cur, Sprite* sprite) {
    Coords c = getSpritePosition(sprite, actor.coords);
    actorEraseDestroyed(cur, sprite, c);
}

void drawAnimSprite(State* cur, Sprite* sprite) {
    Coords c = actorUpdate(cur, sprite);
    SpriteData sd = {
        .coords = c,
        .height = sprite->height,
        .width = sprite->width,
        .spritedata = sprite->data
    };
    maskSprite(NULL, &sd);
}

void drawAlien(State* cur){
    byte spriteidx = cur->spriteIndex;
    cur->spriteIndex = (cur->spriteIndex & 0xc0) | 0x3;
    updateAndEraseActor(cur);
    colorizeSprite(cur);
    cur->spriteIndex = spriteidx;
}