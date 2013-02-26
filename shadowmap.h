
#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "state.h"
#include "rendertarget.h"

class Shadowmap : public RenderTarget
{
private:

public:
    State state;

    Shadowmap();
    ~Shadowmap();
};

#endif

