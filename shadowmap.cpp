
#include <GL3/gl3w.h>

#include "shadowmap.h"

Shadowmap::Shadowmap() : RenderTarget(512)
{
    File statefile = File::read("states/anishadow.xml");
    state.read(statefile);
    state.link();

}

Shadowmap::~Shadowmap()
{
    state.unlink();
}
