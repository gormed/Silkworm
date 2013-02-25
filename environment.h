
/*

    environment.h

    class for static environments

*/

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <list>

#include "state.h"
#include "array.h"
#include "collada.h"


#define ENVLIGHTBOUNCES 1   // how many bounces of light to use when baking lightmaps
#define LIGHTMAPSIZE 128     // size of each lightmap
#define LIGHTMAPRTSIZE 64    // size of the lightmap render target


//
// a single environment component
// this allows multiple instances of the same Geometry
//

struct EnvironmentModelInstance
{
    Matrix transform;
    Matrix rotation;
    Geometry *geo;

};

typedef std::list<EnvironmentModelInstance> EnvironmentModelInstanceList;

//
// environment class
// responsible for maintaining the environment graphics, their instances,
// importing them from a collada file, and rendering them
//

class Environment
{

private:
    Collada *collada;

    State *state;
    ImageMap images;

    EnvironmentModelInstanceList models;

    Image *lightmaps;


    int readCachedLightmaps(int bounce, unsigned char *pixel);

public:

    //void setState() { state->set(); }
    //State *getState() { return state; }
    //ImageMap& getImages() { return images; }


    GeometryMap& getGeometries() { return collada->geometries; }

    void addModel( EnvironmentModelInstance &model) { models.push_front(model); }

    void render(const Matrix &projection, const Matrix &modelview, int lightmap);

    void bakeLightmaps(int bounce);

    Environment();
    ~Environment();

};

#endif

