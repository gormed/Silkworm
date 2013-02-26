
/*

    post.cpp

    screen-space post processing effect

*/

#include "file.h"
#include "post.h"

Post::Post(int _RTsize, char *stateFilename) : RenderTarget(_RTsize)
{
    RTsize=_RTsize;

    // create the state and array for rendering the effect

    File statefile = File::read(stateFilename);
    state.read(statefile);
    state.link();

    static float data[6*4]={ -1.0f, -1.0f, 0.0f, 0.0f,
                              1.0f, -1.0f, 0.0f, 0.0f,
                             -1.0f,  1.0f, 0.0f, 0.0f,
                              1.0f, -1.0f, 0.0f, 0.0f,
                              1.0f,  1.0f, 0.0f, 0.0f,
                             -1.0f,  1.0f, 0.0f, 0.0f };

    array.create(&state,6,data);

    array.link();

}

Post::~Post()
{
    state.unlink();
}



void Post::draw(float targetwidth, float targetheight, float sourcewidth, float sourceheight)
{
    state.set();
    RTcolor.use(0);
    glGenerateMipmap(GL_TEXTURE_2D);

    glUniform2f(glGetUniformLocation(state.program,"invtargetsize"),1.0f/targetwidth, 1.0f/targetheight);
    glUniform2f(glGetUniformLocation(state.program,"invsourcesize"),1.0f/sourcewidth, 1.0f/sourceheight);

    array.draw();
}

