
/*

    state.cpp

    class for creating and setting opengl render states

*/


#include <GL3/gl3w.h>

#include <exception>
#include <sstream>
#include <iostream>

#include "state.h"
#include "xml.h"
#include "log.h"

State::State()
{
    for(int i=0;i<State::TOTALSHADERS;i++) shadertext[i]=NULL;
    for(int i=0;i<State::MAXSYMBOLS;i++) symname[i]=NULL;
    for(int i=0;i<State::MAXSYMBOLS;i++) uniformname[i]=NULL;
    flags=0;
    symcount=0;
    uniformcount=0;
}

void State::read(File &file)
{
    static const char *shadernames[]={"vertex","control","eval","geometry","fragment"};
    static const char *flagnames[]={"earlyz","zwrite","zreverse","blend"};
    static const int flagvalues[]={FLAG_EARLYZ,FLAG_ZWRITE,FLAG_ZREVERSE,FLAG_BLEND};

    NodeMap::iterator p;
    int i,l;

    XML xml = XML::readfile(file);

    NodeMap shaders = xml.get("shader");

    for (p=shaders.begin();p!=shaders.end();++p)
    {
        for(i=0;i<5;i++)
        if ((p->second)["type"]==shadernames[i])
        {
            if (shadertext[i]) throw(1);

            l=(p->second).value().length()+1;
            shadertext[i]=new char[l+1];
            memcpy(shadertext[i],(p->second).value().c_str(),l);
            shadertext[i][l]=0;
        }
    }

    NodeMap lflags = xml.get("flag");

    for(p=lflags.begin();p!=lflags.end();++p)
    {
        for(i=0;i<4;i++)
        if((p->second)["name"]==flagnames[i])
        {
            flags|=flagvalues[i];
        }
    }

    NodeMap symbols = xml.get("symbol");

    for (p=symbols.begin();p!=symbols.end();++p)
    {
        if(symcount>MAXSYMBOLS) throw(1);

        l=(p->second)["name"].length()+1;
        symname[symcount]=new char [l];
        memcpy(symname[symcount],(p->second)["name"].c_str(),l);
        std::stringstream((p->second)["length"]) >> length[symcount];
        symcount++;

    }

    NodeMap uniforms = xml.get("uniform");

    for (p=uniforms.begin();p!=uniforms.end();++p)
    {
        if (uniformcount>=MAXSYMBOLS) throw(1);

        l=(p->second)["name"].length()+1;
        uniformname[uniformcount]=new char [l];
        memcpy(uniformname[uniformcount],(p->second)["name"].c_str(),l);
        uniformcount++;
    }
}

void State::kill()
{
    int i;

    for (i=0;i<State::TOTALSHADERS;i++) if (shadertext[i]) delete []shadertext[i];
    for (i=0;i<State::MAXSYMBOLS;i++) if (symname[i]) delete []symname[i];
    for (i=0;i<State::MAXSYMBOLS;i++) if (uniformname[i]) delete []uniformname[i];
}

void State::enable(int cap, int state)
{
    if(state) glEnable(cap); else glDisable(cap);
}


int State::compileshader(const char *shadertext, int shadertype)
{
    static char buffer[512];

    int didcompile;

    unsigned int shader = glCreateShader(shadertype);

    glShaderSource(shader, 1, &shadertext, 0);
    glCompileShader(shader);

    glGetShaderiv(shader,GL_COMPILE_STATUS,&didcompile);

    if (!didcompile)
    {
        glGetShaderInfoLog(shader, 512, NULL, buffer);

        Log::log() << "glCompileShader() failed.<br>";
        Log::code(shadertext);
        Log::log() << "shader infolog:<br>";
        Log::log() << buffer << "<br>";

        throw(1);
    }

    return shader;
}


void State::link()
{
    static char buffer[512];

    int didlink;
    int i;

    for (i=0;i<TOTALSHADERS;i++)
    if (shadertext[i])
    {
        shader[i] = State::compileshader(shadertext[i], shaderglmap[i]);
    }
    else
    {
        shader[i] = 0;
    }

    program = glCreateProgram();

    for (i=0;i<TOTALSHADERS;i++)
    if (shader[i])
    {
        glAttachShader(program, shader[i]);
    }


    for (i=0;i<symcount;i++) glBindAttribLocation(program,i,symname[i]);

    //glBindFragDataLocation(program,0,"frag");

    glLinkProgram(program);

    glGetProgramiv(program,GL_LINK_STATUS,&didlink);

    if(!didlink)
    {
        glGetProgramInfoLog(program, 512, NULL, buffer);

        Log::log() << "glLinkProgram() failed." << "<br>program infolog:<br>";
        Log::code(buffer);

        throw(1);

        return;
    }

    for (i=0;i<uniformcount;i++) uniformlocation[i] = glGetUniformLocation( program, uniformname[i] );
}

void State::unlink()
{
    int i;

    glDeleteProgram(program);

    for (i=0;i<TOTALSHADERS;i++)
        glDeleteShader(shader[i]);
}


void State::set() const
{
    enable(GL_DEPTH_TEST, flags & FLAG_EARLYZ);
    enable(GL_BLEND,      flags & FLAG_BLEND);

    glDepthMask( (flags & FLAG_ZWRITE) ? 1 : 0);
    glDepthFunc( (flags & FLAG_ZREVERSE) ? GL_GREATER : GL_LESS);

    glUseProgram(program);
}

int State::getSymcount() const
{
    return symcount;
}

int State::getLength(int i) const
{
    return length[i];
}

int State::getUniformLocation(int i) const
{
    return uniformlocation[i];
}

