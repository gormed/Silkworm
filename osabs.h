
#ifndef OSABS_H
#define OSABS_H

// these functions are provided by osabs.cpp

void frame_hook();
int system_hook(int *quit);
int create_context();
int kill_context();

// these functions are called by osabs.cpp
// and must be implemented in the main program code

int renderloop();
void idle();
int keydown(int keycode);
int mousemove(int x, int y);
void windowsize(int w, int h);

#endif



