
/*

    sky.cpp

    sky renderer

*/

#include "file.h"
#include "sky.h"

Sky::Sky()
{
    File statefile = File::read("states/sky.xml");
    state.read(statefile);
    state.link();

    File skyfile = File::read("models/sky.dae");
    collada.state=&state;
    collada.read(skyfile);

    File imagefile = File::read("images/sky.png");
    image.read(imagefile);
    image.link();
}

Sky::~Sky()
{
    state.unlink();
    image.unlink();
}

void Sky::draw(Matrix &projection, Matrix &modelview)
{
    state.set();

    state.uniformMatrix("projection",projection);
    state.uniformMatrix("modelview",modelview);

    image.use(0);
    collada.geometries["Cylinder"].meshes[0].array.draw();

}
