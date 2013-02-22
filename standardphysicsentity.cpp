
/*

    standardphysicsentity.cpp

    part of the entity system

*/


#include "entity.h"

void StandardPhysicsEntity::step()
{
    int t,i;

    if(movementState != FIX)
    {

        // check collision with ground first
        int co[3]= { 1,0,2 };

        for(i=0;i<3;i++)
        {
            collisionState = collide(this);

            if (collisionState & (1<<(co[i]*2)) )
            {
                if (vel.e[co[i]]<0) vel.e[co[i]]=0;

                t=(int)pos.e[co[i]];
                pos.e[co[i]]=(float)t-bba.e[co[i]]+EPSILON;
            }

            if (collisionState & (1<<(co[i]*2+1)) )
            {
                if (vel.e[co[i]]>0) vel.e[co[i]]=0;

                t=(int)pos.e[co[i]]+1;
                pos.e[co[i]]=(float)t-bbb.e[co[i]]-EPSILON;
            }
        }

        vel += acc;
        pos += vel;

        acc = Vector(0.0f,0.0f,0.0f);

        if (movementState == FREEFALL)
        {
            acc.e[1] = -0.025f; // gravity accelerates!
            vel *= 0.98f;       // friction is low in air
        }
        else
        {
            vel *= 0.8f;        // friction is high on ground
            if (vel.dot(vel)<=0.05f*0.05f) vel = Vector(0.0f,0.0f,0.0f);    // friction stops
        }
    }

    collisionState=collide(this);

    collisionResponse();
}
