
/*

    state.h

    class for creating and setting opengl render states

*/

#ifndef STATE_H
#define STATE_H

#include <GL3/gl3w.h>

#include "resource.h"

static const int shaderglmap[] = { GL_VERTEX_SHADER,
                               GL_TESS_CONTROL_SHADER,
                               GL_TESS_EVALUATION_SHADER,
                               GL_GEOMETRY_SHADER,
                               GL_FRAGMENT_SHADER };

// a renderstate consists of

// - vertex, [tesselation control, tesselation evaluation, geometry], and fragment shader texts
// - one or more symbol definitions
// - zero or more uniform definitions
// - flags for early z test, z wrting, reverse z, alpha testing, and blending

// typical usage of the State resource:

// (1) load data from a xml file by calling read()

// (2) compile the shaders and load them to the graphics board by calling link()

// (3) you may now call kill() to release memory for the shader texts,
//     as they are no longer needed

// (4) you may now use this state to compile Arrays against them
//     and use the state for rendering by calling set()

// (5) call unlink() to cleanup

class State : Resource
{
    public:

    static const int SHADER_VERTEX      = 0;
    static const int SHADER_CONTROL     = 1;
    static const int SHADER_EVAL        = 2;
    static const int SHADER_GEOMETRY    = 3;
    static const int SHADER_FRAGMENT    = 4;

    static const int TOTALSHADERS       = 5;
    static const int MAXSYMBOLS         = 16;

    static const int FLAG_EARLYZ        = 0x01;
    static const int FLAG_ZWRITE        = 0x02;
    static const int FLAG_ZREVERSE      = 0x08;
    static const int FLAG_ALPHA_TEST    = 0x10;
    static const int FLAG_BLEND         = 0x20;

    //private: // this should be un-commented!

    unsigned int shader[5];

    unsigned int flags;
    unsigned int program;

    char *shadertext[8];

    int symcount;
    int location[MAXSYMBOLS];
    char *symname[MAXSYMBOLS];
    int length[MAXSYMBOLS];

    int uniformcount;
    char *uniformname[MAXSYMBOLS];
    int uniformlocation[MAXSYMBOLS];

    public:

    State();

    void read (File &file);
    void kill ();

    void link ();
    void unlink ();

    int getSymcount () const;
    int getLength (int i) const;

    int getUniformLocation (int i) const;

    void set () const;

    static void enable(int cap, int state);
    static int compileshader(const char *shadertext, int shadertype);


};


#endif



