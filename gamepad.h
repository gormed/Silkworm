
/*

    gamepad.h

    gamepad support

*/

#ifndef GAMEPAD_H
#define GAMEPAD_H

struct gamepad
{
	float x,y,z,u,v;
	int b[8];
};


int gamepad_create();
int gamepad_query(gamepad &ji);

#endif

