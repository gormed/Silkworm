
/*
    rendertarget.h

    creating and maintaining render targets

*/

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "image.h"

class RenderTarget
{
protected:
    Image RTcolor;
    Image RTdepth;

    int RT;
    int RTsize;


public:

    RenderTarget(int _RTsize);
    ~RenderTarget();

    Image *getDepthImage() { return &RTcolor; }

    void begin();
    void end();


};

#endif



