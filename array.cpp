
/*

    array.cpp

    class for creating and rendering opengl vertex arrays

*/

#include <GL3/gl3w.h>

#include "array.h"

Array::Array()
{
    usestate=NULL;
    data=NULL;
}

void Array::link()
{
    float *dataptr;

    int i;

    State *st = usestate;

    if(!st) return;
    if(!data) return;

    dataptr=data;

    glGenVertexArrays(1,&array);
    glBindVertexArray(array);

    glGenBuffers(st->getSymcount(), buffer);

    for (i=0;i<st->getSymcount();i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer[i]);
        glVertexAttribPointer(i,st->getLength(i),GL_FLOAT,false,0,0);
        glBufferData(GL_ARRAY_BUFFER, 4 * st->getLength(i) * elements, dataptr, GL_STATIC_DRAW);

        dataptr += st->getLength(i) * elements;
    }

    for (i=0; i < st->getSymcount(); i++) glEnableVertexAttribArray(i);
    for (   ; i < State::MAXSYMBOLS; i++) glDisableVertexAttribArray(i);

    glBindVertexArray(0);
}

void Array::unlink()
{
    glDeleteBuffers(usestate->getSymcount(), buffer);
    glDeleteVertexArrays(1, &array);

    array=0;
}

void Array::draw() const
{
    glBindVertexArray(array);
    glDrawArrays(GL_TRIANGLES, 0, elements);
    glBindVertexArray(0);
}

