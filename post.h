
/*

    post.h

    screen-space post processing effect

*/


#ifndef POST_H
#define POST_H

#include "state.h"
#include "array.h"
#include "image.h"

class Post
{
private:

    State state;
    Array array;

    Image RTcolor;
    Image RTdepth;

    int RT;
    int RTsize;

public:

    Post(int RTsize, char *stateFilename);
    ~Post();

    void begin();
    void end();

    void draw(float targetwidth, float targetheight, float sourcewidth, float sourceheight);

};

#endif
