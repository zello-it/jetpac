#pragma once
#include <stdbool.h>

enum KeyboardResult {
    None = 0xff,    // 1111 1111
    Left = 0xef,    // 1110 1111
    Right = 0xf7,   // 1111 0111
    Thrust = 0xfd, // 1111 1101
    Fire = 0xfe,   // 1111 1110 (ma perché??)
    Hover = 0xdf   // 1101 1111 per simmetria...
};

enum Keys {
    keyPause = 0,
    keyOne,
    keyTwo,
    keyThree,
    keyFour,
    keyFive
};

enum KeyboardResult readInputLR();
enum KeyboardResult readInputThrust();
enum KeyboardResult readInputFire();
enum KeyboardResult readInputHover();

bool isKeyDown(enum Keys key);
bool isKeyPressed(enum Keys key);
