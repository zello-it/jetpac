#include "sprite.h"
#include "video.h"
#include <stdbool.h>

static void actorEraseMovedSprite(SpriteData* oldSprite, SpriteData* newSprite) {
    sbyte diff = oldSprite->coords.y - newSprite->coords.y;
    bool b = false;
    switch(byteSgn(diff)) {
    case 0:
        oldSprite->height = actor.gfxDtaHeight;
        actor.spriteHeight = actor.gfxDtaHeight = 0;
        maskSprite(oldSprite, newSprite);
        break;
    case -1:
        diff = byteAbs(diff);
        if(actor.gfxDtaHeight < diff) {
            newSprite->height = diff;
            oldSprite->height = actor.gfxDtaHeight;
            actor.spriteHeight = actor.gfxDtaHeight = 0;
            maskSprite(oldSprite, newSprite);
        } else {
            actor.gfxDtaHeight = actor.gfxDtaHeight - diff;
            maskSprite(oldSprite, newSprite);
        }
        break;
        //b = true;
        //fallthrough
    case 1: 
        if(actor.gfxDtaHeight < diff) {
            oldSprite->height = diff;
            actor.gfxDtaHeight = actor.gfxDtaHeight = 0;    
            maskSprite(oldSprite, newSprite);
        } else {
            actor.spriteHeight -= diff;
            oldSprite->height = diff;
            maskSprite(oldSprite, newSprite);
        }
         break;
    } // end switch
}


static void actorEraseDestroyed(State* state, Sprite* sprite, Coords coords) {
    byte c = actor.height;
    actor.gfxDtaHeight = 0;
    actor.height = 0;
    SpriteData sd = {
        .spritedata = sprite->data,
        .coords = coords,
        .height = c,
        .width = actor.width
    };
    maskSprite(&sd, NULL);
}

static void writeSprite(SpriteData* sd, enum Operator op) {
    while(sd->height--) {
        for(byte col = 0; col < sd->width; ++col) {
            byte out = *sd->spritedata++;
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

void updateAndEraseActor(State* state) {
    Sprite* s = getSpriteAddress(state->x & 0x6, state->spriteIndex);
    actorUpdate(state, s);
    SpriteData newSprite = {
        .spritedata = s->data,
        .height = s->height,
        .coords = (Coords){.x = state->x, .y = state->y},//actorCoords,
        .width = s->width
    };
    Sprite* spriteActor = getSpriteAddress(actor.x & 0x6, actor.spriteIndex);
    SpriteData oldSprite = {
        .spritedata = spriteActor->data,
        .height = spriteActor->height,
        .coords = actor.coords,
        .width = spriteActor->width
    };
    //Coords act = getSpritePosition(spriteActor, actorCoords);
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
    if(maskSprite && newSprite && (maskSprite->coords.x != newSprite->coords.x)) {
        printf("Deleting %x @%d, %d h = %d\n", (uint) maskSprite->spritedata, maskSprite->coords.x, maskSprite->coords.y, maskSprite->height);
        printf("Writing %x @%d, %d h = %d\n", (uint) newSprite->spritedata, newSprite->coords.x, newSprite->coords.y, newSprite->height);
        printf("******\n");
    }
    lockVideo();
    if(maskSprite && maskSprite->height) {  
        writeSprite(maskSprite, AND);
    }
    if(newSprite && newSprite->height) {
        writeSprite(newSprite, OR);
    }
    unlockVideo();
}


Sprite* getSpriteAddress(byte x, byte header) {
    if(header & 0x40) {  // maybe 0x20... Qui entra x come offset. I bytes alti escono 
        x |= 0x8;        // per effetto dello shift (o del rlca maskato con f0)
    }
    --header;
    byte index = (header << 4) | x; // il mask (& 0xf0) non serve, non stiamo ruotando
    index >>= 1; // word to index
//    printf("Asking for %x, returned %d\n", header, index);
    return spriteTable[index];
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


Coords actorUpdate(State* state, Sprite* sprite) {
    actorCoords = (Coords) {.x = state->x + sprite->xoffset, .y = state->y};
    actor.width = sprite->width;
    actor.gfxDtaHeight = actor.height = sprite->height;
    return actorCoords;
}

