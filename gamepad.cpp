
/*

    gamepad.h

    gamepad support

    os-specific (windows 32-bit)

*/

#include <windows.h>

#include "gamepad.h"

int usedev=-1;

JOYCAPS caps;

int gamepad_init()
{
	return joyGetDevCaps(usedev,&caps,sizeof(caps));
}

int gamepad_create()
{
	JOYINFO joyinfo;

    if(joyGetNumDevs()==0) return 0;
    if (joyGetPos(JOYSTICKID1,&joyinfo) != JOYERR_UNPLUGGED) { usedev=JOYSTICKID1; gamepad_init(); return 1; }
    if (joyGetPos(JOYSTICKID2,&joyinfo) != JOYERR_UNPLUGGED) { usedev=JOYSTICKID2; gamepad_init(); return 1; }
	return 0;
}

int gamepad_query(gamepad &ji)
{
	if (usedev==-1) return 0;
	JOYINFOEX j;
	memset(&j,0,sizeof(JOYINFOEX));
	j.dwSize=sizeof(JOYINFOEX);
	j.dwFlags=JOY_RETURNALL;
	joyGetPosEx(usedev,&j);

	ji.x=(((float)j.dwXpos-(float)caps.wXmin)/((float)caps.wXmax-(float)caps.wXmin))*2.0f-1.0f;
	ji.y=(((float)j.dwYpos-(float)caps.wYmin)/((float)caps.wYmax-(float)caps.wYmin))*2.0f-1.0f;
	ji.z=(((float)j.dwZpos-(float)caps.wZmin)/((float)caps.wZmax-(float)caps.wZmin))*2.0f-1.0f;
	ji.u=0;ji.v=0;
	if (j.dwPOV!=JOY_POVCENTERED)
	{
		if (j.dwPOV>0 && j.dwPOV<18000) ji.u=-1;
		if (j.dwPOV>18000) ji.u=1;
		if (j.dwPOV>9000 && j.dwPOV<27000) ji.v=-1;
		if (j.dwPOV<9000 || j.dwPOV>27000) ji.v=1;
	}

	if (ji.x>-0.1f && ji.x<0.1f) ji.x=0;
	if (ji.y>-0.1f && ji.y<0.1f) ji.y=0;
	if (ji.z>-0.1f && ji.z<0.1f) ji.z=0;
	ji.b[0]=j.dwButtons&JOY_BUTTON1;
	ji.b[1]=j.dwButtons&JOY_BUTTON2;
	ji.b[2]=j.dwButtons&JOY_BUTTON3;
	ji.b[3]=j.dwButtons&JOY_BUTTON4;
	ji.b[4]=j.dwButtons&JOY_BUTTON5;
	ji.b[5]=j.dwButtons&JOY_BUTTON6;
	ji.b[6]=j.dwButtons&JOY_BUTTON7;
	ji.b[7]=j.dwButtons&JOY_BUTTON8;

	return 1;
}
