
/*

    array.cpp

    class for creating and rendering opengl vertex arrays

*/

#ifndef ARRAY_H
#define ARRAY_H

#include "resource.h"
#include "state.h"

// typical usage of the Array resource:

// (1) point usestate to a valid and already linked renderstate
//     fill data with values
//     set elements to the number of elements (triangles) to be rendered

// (2) call link

// (3) you can now delete the contents of data if you want,
//     they are now longer needed in system memory as they have been
//     transfered to the gpu

// (4) use the array by calling draw()

// (5) clean up by calling unlink()

class Array : Resource
{
    private:

    unsigned int array;
    unsigned int buffer[8];
    int mode;

    public:

    State *usestate;

    unsigned int elements;
    float *data;

    Array();

    void link ();
    void unlink ();

    void draw() const;


};

#endif

