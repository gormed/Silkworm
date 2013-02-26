
/*

    post.cpp

    screen-space post processing effect

*/

#include "file.h"
#include "post.h"

Post::Post(int _RTsize, char *stateFilename)
{
    RTsize=_RTsize;

    // create the render target for the effect input

    RTcolor.create ( RTsize, RTsize, Image::RGBA, NULL );
    RTdepth.create ( RTsize, RTsize, Image::DEPTH, NULL );

    RTcolor.link();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    RTdepth.link();

    glGenFramebuffers(1, (GLuint*)&RT);
    glBindFramebuffer(GL_FRAMEBUFFER, RT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, RTdepth.getTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RTcolor.getTexture(), 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    RTcolor.unlink();
    RTdepth.unlink();

    glDeleteFramebuffers(1,(GLuint*)&RT);

    state.unlink();
}

void Post::begin()
{
    glBindFramebuffer(GL_FRAMEBUFFER, RT);
    glViewport(0,0,RTsize,RTsize);
}

void Post::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

