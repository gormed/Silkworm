
/*
    rendertarget.cpp

    creating and maintaining render targets

*/

#include <GL3/gl3w.h>

#include "rendertarget.h"

RenderTarget::RenderTarget(int _RTsize)
{
    RTsize = _RTsize;

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
}

RenderTarget::~RenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    RTcolor.unlink();
    RTdepth.unlink();

    glDeleteFramebuffers(1,(GLuint*)&RT);
}

void RenderTarget::begin()
{
    glBindFramebuffer(GL_FRAMEBUFFER, RT);
    glViewport(0,0,RTsize,RTsize);
}

void RenderTarget::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
