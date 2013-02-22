
/*

    image.cpp

    class for loading opengl textures from png files

*/

#ifndef IMAGE_H
#define IMAGE_H

#include "resource.h"

// typical usage of the image resource

// (1) load an image from a png file with read()

// (2) call link()

// (3) you may now delete the data in system memory by calling kill()

// (4) you may now call use() to bind the texture to a texture unit

// (5) cleenup by calling unlink();

class Image : Resource
{
    private:

    int width;
    int height;
    int mode;
    unsigned char *data;
    int texture;

    public:

    Image();

    virtual void read (File &file);
    virtual void kill ();

    virtual void link ();
    virtual void unlink ();

    void use (int unit) const;

    static const int ALPHA = 1;
    static const int RGB = 3;
    static const int RGBA = 4;
    static const int DEPTH = 5;
    static const int KEYED = 6;

};


#endif


