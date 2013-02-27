
/*

    entity

    class definitions for the entity system

*/


#ifndef ENTITY_H
#define ENTITY_H

#include "vector.h"

#define COLLISION_LEFT          1
#define COLLISION_RIGHT         2
#define COLLISION_BOTTOM        4
#define COLLISION_TOP           8
#define COLLISION_BACK          16
#define COLLISION_FRONT         32
#define COLLISION_CLIMB_LEFT    64
#define COLLISION_CLIMB_RIGHT   128
#define COLLISION_CLIMB_BACK    256
#define COLLISION_CLIMB_FRONT   512


enum MovementState { LEFTWALL, RIGHTWALL, FRONTWALL, BACKWALL, CEIL, FLOOR, FREEFALL, FIX };

//
// base entity class
// all game objects should be derived from this class
//

class Entity
{
    public:

    Vector pos;                             // position
    Vector vel;                             // velocity
    Vector acc;                             // acceleration
    Vector bba,bbb;                         // bounding left,back,top and right,front,bottom

    int collisionState;                     // last detected collisions, flags as in defines
    MovementState movementState;            // mode of movement

    int hp,ac,dc;                           // hit points, armor class, damage class

    virtual void render() = 0;

    virtual void collisionResponse() {};
    virtual void step() = 0;

    virtual void control(int *keyStates, int *lastKeyStates) {};
};



int collide(Entity *e);                    // this is a temporary hack to get it working quickly

//
// entity with standard physics
// implements simple newtonian physics (position,velocity,acceleration)
// the entity stops when it hits a wall
// the entity is accelerated by gravity, but not if it rests on a floor
//

class StandardPhysicsEntity : public Entity
{
    public:

    void step();
};


//
// animation control information for the player character
//

struct AniControl
{
    int frame,lastframe;
    float ip;
    float rotation;
    float positionoffset;
};

//
// the druid entity implements the special actions
// of the player character
//

enum DruidState { DRUID, SPIDER, BIRD };

enum DruidPrimaryActionState { STANDING, RUNNING, SPEEDING, DUCKINGDOWN, COVERING, STANDINGUP, JUMPING, FALLING, CLIMBING };

enum DruidSecondaryActionState { IDLE, FIRING, MELEE, TRANSFORMING };

class DruidEntity : public StandardPhysicsEntity
{
    public:

    AniControl aniControl;

    DruidState druidState;                                  // the druids current animal form
    DruidState oldDruidState;                               // the druids previous animal form, if he is currently transforming
    DruidPrimaryActionState druidPrimaryActionState;        // movement actions
    DruidSecondaryActionState druidSecondaryActionState;    // combat/special actions

    int actionDuration;       // some actions last a number of frames, in this case, actionDuration counts down to zero
    int actionDirection;      // some actions are directed, the direction is stored here

    Vector intendedDirection; // not actual Movement, but where the player wants to go

    DruidEntity();

    void collisionResponse();

    void render()
    {
        // todo: render the druid
    }

    void control(int *keyStates, int *lastKeyStates, Vector cameraDirection);
};

#endif
