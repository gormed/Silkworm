
/*

    post.h

    screen-space post processing effect

*/


#ifndef POST_H
#define POST_H

#include "state.h"
#include "array.h"
#include "rendertarget.h"

class Post : public RenderTarget
{
private:

    State state;
    Array array;

public:

    Post(int _RTsize, char *stateFilename);
    ~Post();

    void draw(float targetwidth, float targetheight, float sourcewidth, float sourceheight);


};

#endif
