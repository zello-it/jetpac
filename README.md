# jetpac
A porting of the old z80 game jetpac to modern hardware, based on the disassembly of the original code (thanks to mrcook, https://github.com/mrcook/jetpac-disassembly).

It uses raylib (https://raylib.com) as a graphic and input library, it should be easy to substitute with any other gfx library (the display is handled in video.h/video.c, the keyboard is handled in keyboard.h/keyboard.c).

## todo
Sound is not implemented (it should be pretty easy, but zx spectrum original timings should be taken into account).

## compile
Ordinary cmake procedure: it should compile under linux, macos (but openGL, which is used by raylib, is marked as deprecated) and windows.
