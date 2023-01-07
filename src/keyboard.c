#include "keyboard.h"
#include "data.h"
#include <raylib.h>

int keys[] = {
    KEY_LEFT_SHIFT,
    KEY_ONE,
    KEY_TWO,
    KEY_THREE,
    KEY_FOUR,
    KEY_FIVE
};

int keyLeft[] = {
    KEY_Z, KEY_C, KEY_B, KEY_M
};
int keyRight[] = {
    KEY_X, KEY_V, KEY_N, KEY_COMMA
};
int keyThrust[] = {
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T,
    KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P
};
int keyFire[] = {
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G,
    KEY_H, KEY_J, KEY_K, KEY_L
};
int keyHover[] = {
    KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE,
    KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE
};

bool anyPressed(int* keylist, size_t sz) {
    for(size_t i = 0; i < sz; ++i) {
        if(IsKeyDown(keylist[i]))
            return true;
    }
    return false;
}

enum KeyboardResult readInputLR() {
    if(anyPressed(keyLeft, array_sizeof(keyLeft)))
        return Left;
    if(anyPressed(keyRight, array_sizeof(keyRight)))
        return Right;
    return None;
}
enum KeyboardResult readInputThrust() {
    if(anyPressed(keyThrust, array_sizeof(keyThrust)))
        return Thrust;
    return None;
}
enum KeyboardResult readInputFire() {
    if(anyPressed(keyFire, array_sizeof(keyFire)))
        return Fire;
    return None;
}
enum KeyboardResult readInputHover() {
    if(anyPressed(keyHover, array_sizeof(keyHover)))
        return Hover;
    return None;
}

bool isKeyDown(enum Keys key) {
    return IsKeyDown(keys[key]);
}
bool isKeyPressed(enum Keys key) {
    return IsKeyPressed(keys[key]);
}
