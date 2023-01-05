#include "data.h"
#include "menu.h"
#include "video.h"
#include <stdbool.h>
#include <raylib.h>

static struct {
    byte line;
    Attrib attrib;
    const char* text;
} menuEntries[] = {
    {
        .line = 0x20,
        .attrib.attrib = 0x47,
        .text = "Jetpac Game Selection"
    },
    {
        .line = 0x38,
        .attrib.attrib = 0x47,
        .text = "1   1 Player Game"
    },
    {
        .line = 0x48,
        .attrib.attrib = 0x47,
        .text = "2   2 Player Game"
    },
    {
        .line = 0x58,
        .attrib.attrib = 0x47,
        .text = "3   Keyboard"
    },
    {
        .line = 0x68,
        .attrib.attrib = 0x47,
        .text = "4   Joystick"
    },
    {
        .line = 0x98,
        .attrib.attrib = 0x47,
        .text = "5   Start Game!"
    }
};

void menuDrawEntry(byte nr) {
    textOutAttrib(
      (Coords){.x = 0x30, .y = menuEntries[nr].line},
      menuEntries[nr].text,
      menuEntries[nr].attrib
    );
}

void menuDrawEntries(){
    
     for(int i = 0; i < array_sizeof(menuEntries); ++i) {
        menuDrawEntry(i);
    }
}

void check1_4() {
    if(IsKeyPressed(KEY_ONE)){
       gameOptions.players = 0;
    }
    else if(IsKeyPressed(KEY_TWO)) {
        gameOptions.players = 1;
    }
    if(IsKeyPressed(KEY_THREE)) {
        gameOptions.input = 0;
    } else if(IsKeyPressed(KEY_FOUR)) {
        gameOptions.input = 1;
    }
}

bool check5(void) {
    return(IsKeyPressed(KEY_FIVE));
}

void menuScreen(void) {

    while(true) {
        checkTermination();
        byte players = 1;
        byte control = 1;
        if(gameOptions.players == 1) {
            players = 0;
        }
        if(gameOptions.input ==1) {
            control = 0;
        }
        menuEntries[1].attrib.flash = players;
        menuEntries[2].attrib.flash = ~players;
        menuEntries[3].attrib.flash = control;
        menuEntries[4].attrib.flash = ~control; 
        menuDrawEntries();
        check1_4();
        if(check5())
            break;
    }
}