
/*

    text.h

    class for very simple text rendering

*/

#ifndef TEXT_H
#define TEXT_H

class Text
{
    public:

    static void init();
    static void deinit();
    static void update();
};

// hook for passing text to the renderer
// this function should be implemented by the game program
// and return a 64 by 32 char array

const char* console();

#endif



