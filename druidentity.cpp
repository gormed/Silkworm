
/*

    druidentity.cpp

    part of the entity system

*/


#include "entity.h"

DruidEntity::DruidEntity()
{
    intendedDirection = Vector (0.0f,0.0f,0.0f);

    bba = Vector(-0.25f,-0.9f,-0.25f);  // initial size of the druid
    bbb = Vector(0.25f,0.9f,0.25f);

    vel = Vector(0.0f,0.0f,0.0f);
    acc = Vector(0.0f,0.0f,0.0f);

    pos = Vector(2.0f,2.0f,2.0f);

    druidState = DRUID;
    oldDruidState = DRUID;
    druidPrimaryActionState = STANDING;
    druidSecondaryActionState = IDLE;
}

void DruidEntity::collisionResponse()
{
    switch (druidState)
    {
    case SPIDER:

        /*for (i=0;i<6;i++)           // make the spider crawl along the walls
        if (collisionState & i)         //!!this code is severly wrong...
        {
            movementState = MovementState[i];
        }*/

        break;

    case BIRD:

        oldDruidState=BIRD;         // make the bird transform back upon impact
        druidState=DRUID;
        druidSecondaryActionState=TRANSFORMING;
        actionDuration=10;

        // purposely no break here (check if the bird has landed safely)!

    case DRUID:

        // when a wall is hit, get out of run mode

        if ( (collisionState & (1|2|16|32) ) && movementState==FLOOR)
        {
            int t=collisionState;
            if (druidPrimaryActionState==SPEEDING||druidPrimaryActionState==RUNNING)
                druidPrimaryActionState=STANDING;
        }

        // when falling to the floor, land

        if( (collisionState & COLLISION_BOTTOM) &&  movementState==FREEFALL)
        {
            movementState = FLOOR;
            druidPrimaryActionState = STANDING;

            //druidPrimaryActionState = COVERING;
        }

        // when walking of an edge, begin to fall

        else if (!(collisionState & COLLISION_BOTTOM) && movementState==FLOOR)
        {
            movementState = FREEFALL;
            druidPrimaryActionState = FALLING;
        }

        break;

    }
}

void DruidEntity::control(int *keyStates, int *lastKeyStates)
{
    int i;

    // running / steering in free fall / pulling up

    intendedDirection=Vector(0.0f,0.0f,0.0f);


    if (keyStates[0]) intendedDirection.e[0]-=1.0f;
    if (keyStates[1]) intendedDirection.e[0]+=1.0f;
    if (keyStates[2]) intendedDirection.e[2]-=1.0f;
    if (keyStates[3]) intendedDirection.e[2]+=1.0f;



    if (actionDuration>0) actionDuration--;

    switch (druidPrimaryActionState)
    {
    case STANDING:
    case RUNNING:

        if (  keyStates[0]||keyStates[1]||keyStates[2]||keyStates[3] )
        {
            intendedDirection.normalize();
            intendedDirection*=0.08f;

            if (vel.dot(vel) <= 0.1f*0.1f) acc+=intendedDirection;  // only go so fast

            if (druidPrimaryActionState != RUNNING)
            {
                actionDuration = 30; // hold to run
                druidPrimaryActionState = RUNNING;
            }
            else if (actionDuration==0)
            {
                druidPrimaryActionState=SPEEDING;
            }

        }
        else
        {
            druidPrimaryActionState = STANDING;
        }

        break;

    case SPEEDING:

        if (  keyStates[0]||keyStates[1]||keyStates[2]||keyStates[3] )
        {
            intendedDirection.normalize();
            intendedDirection*=0.08f;

            if (vel.dot(vel) <= 0.2f*0.2f) acc+=intendedDirection;
        }
        else
        {
            druidPrimaryActionState = STANDING;
        }

        break;

    case FALLING:

        acc+=intendedDirection*0.1*0.08f;

        for(i=0;i<4;i++)

        if (keyStates[i] && (collisionState&(64<<i)))
        {
            druidPrimaryActionState = CLIMBING;
            actionDirection = i;
            movementState = FIX;
            actionDuration = 10;
            vel=Vector(0.0f,0.0f,0.0f);
            pos.e[1]=(float)((int)( (pos.e[1]+bba.e[1])*0.5f)*2+2);
        }


        break;

    default:

        ; // nothing

    }

    // jump

    if (keyStates[6]&&!lastKeyStates[6])
    {
        if (druidPrimaryActionState==COVERING && vel.dot(vel)>=0.2f*0.2f)   // duck-jump out of speeding
        {
            movementState=FREEFALL;
            druidPrimaryActionState=FALLING;
            acc.e[1]+=1.0f;
        }

        else if (  druidPrimaryActionState==RUNNING || druidPrimaryActionState==STANDING
                || druidPrimaryActionState==SPEEDING || druidPrimaryActionState==COVERING)
        {
            movementState=FREEFALL;
            druidPrimaryActionState=FALLING;
            acc.e[1]+=0.5f;
        }

        else if (druidPrimaryActionState==FALLING)   // wall-jump
        {
            if (collisionState&(1|2|16|32))
            {
                movementState=FREEFALL;
                druidPrimaryActionState=FALLING;
                acc.e[1]+=0.6f;
            }
            if (collisionState&1) acc.e[0]+=0.3f;
            else if (collisionState&2) acc.e[0]-=0.3;
            else if (collisionState&16) acc.e[2]+=0.3;
            else if (collisionState&32) acc.e[2]-=0.3;
        }
    }

    if(druidPrimaryActionState==FALLING&&!keyStates[6]&&lastKeyStates[6])  // fall out of jump
    {
        if (vel.e[1]>0.0f) vel.e[1]=0.0f;
    }

    // shoot

    if (keyStates[7]&&!lastKeyStates[7])
    {
        if( keyStates[9]) // down
        {
        }
        if( keyStates[10]) // up
        {
        }
    }

    // melee

    if (keyStates[8]&&!lastKeyStates[8])
    {
        if( keyStates[9]) // down
        {
        }
        if( keyStates[10]) // up
        {
        }
    }

    // cover

    if (keyStates[11]&&!lastKeyStates[11])  // duck down
    {
        if (  druidPrimaryActionState==RUNNING || druidPrimaryActionState==STANDING
            ||druidPrimaryActionState==SPEEDING)
        {
            druidPrimaryActionState=DUCKINGDOWN;
            actionDuration=5;
        }
    }


    if(!keyStates[11])  // stand up
    {
        if ( druidPrimaryActionState==COVERING)
        {
            druidPrimaryActionState=STANDINGUP;
            actionDuration=5;
        }
    }

    if (druidPrimaryActionState==DUCKINGDOWN) { bbb.e[1]-=0.9f*0.2f; }
    if (druidPrimaryActionState==STANDINGUP) { bbb.e[1]+=0.9f*0.2f; }

    if (druidPrimaryActionState==DUCKINGDOWN&&actionDuration==0)
    {
        druidPrimaryActionState=COVERING;
        bbb.e[1]=0.0f;
    }

    if (druidPrimaryActionState==STANDINGUP&&actionDuration==0)
    {
        bbb.e[1]=0.9f;
        druidPrimaryActionState=STANDING;
    }

    if (druidPrimaryActionState==CLIMBING&&actionDuration==0)
    {
        druidPrimaryActionState=STANDING;
        movementState=FLOOR;
        switch(actionDirection)
        {
            case 0: pos.e[0]=(float)((int)pos.e[0]); break;
            case 1: pos.e[0]=(float)((int)pos.e[0]+1); break;
            case 2: pos.e[2]=(float)((int)pos.e[2]); break;
            case 3: pos.e[2]=(float)((int)pos.e[2]+1); break;
        }
        pos.e[1]=(float)((int)(pos.e[1]*0.5f)*2)-bba.e[1];
    }


}

