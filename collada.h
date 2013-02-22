
/*

    collada.h

    class for importing collada files


*/

#ifndef COLLADA_H
#define COLLADA_H

#include "xml.h"
#include "array.h"
#include "state.h"
#include "image.h"

struct Geometry;
struct Mesh;

typedef std::map<std::string, Geometry> GeometryMap;
typedef std::map<int, Mesh> MeshMap;
typedef std::map<std::string, Image> ImageMap;


// a Collada object loads all the geometry info from a Collada file
// and creates and links the corresponding opengl vertex arrays

// one render state must be specified for the whole file.

// position / orientation / further properties of objects are not loaded,
// since they are not needed.

// material names are loaded, but not the materials themselves -
// blender does not always correctly save the material definitions into the file,
// and texture file names change between 3d editor and game anyways.

class Collada
{
private:

    void readGeometry(XML &geoxml);

public:

    GeometryMap geometries;

    // which renderstate to use for the models from this file

    State *state;

    Collada() { state = NULL; }

    void read(File &file);


};

class Geometry
{
public:

    // if a geometry object consists of multiple polylist,
    // leave them seperate.

    // almost certainly, materials (textures) will differ,
    // resulting in a opengl state change so having
    // seperate vertex arrays is a good idea.

    MeshMap meshes;

    void draw(ImageMap &materials);
};

struct Mesh
{
    Array array;
    std::string material;
};

#endif

