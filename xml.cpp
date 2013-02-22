
/*

    xml.cpp

    class for loading xml files and accessing the xml data structure

*/

#include "xml.h"

// helper function used to determinine wether a certain character
// is considered to be whitespace in the xml file

inline bool iswhitespace(char c)
{
    return (c==' '||c=='\t'||c=='\r'||c=='\n');
}

// read a single xml tag
// read() calls itself recursively to load nested tags

void XML::read(File &file, int &position)
{

    int start=0,end=0,start2=0;
    int childcount=0;
    char escape;
    char *fc=(char*)file.contents;

    if (fc[position]!='<') throw (1);
    position++;

    start=position;
    while(!iswhitespace(fc[position]))
    {
        if (fc[position]==0) throw (1);
        if (fc[position]=='/') throw (1);
        if (fc[position]=='>') break;
        position++;
    }

    if(start==position) throw (1);

    name=std::string(fc+start,position-start);

    for(;;)
    {
        while (iswhitespace(fc[position]))
        {
            position++;
        }

        if (fc[position]=='/' && fc[position+1]=='>') return;

        if (fc[position]=='>') {position++; break; }
        if (fc[position]=='=') throw(1);
        if (fc[position]==0) throw(1);

        start=position;
        while(fc[position]!='=')
        {
            if (iswhitespace(fc[position])) throw(1);
            if (fc[position]==0) throw (1);
            if (fc[position]=='>') throw(1);
            position++;
        }

        end=position;
        position++;

        if(   fc[position]!='\''
           && fc[position]!='"') throw(1);

        escape=fc[position];
        position++;

        start2=position;
        while(fc[position]!=escape)
        {
            if(fc[position]==0) throw(1);
            position++;
        }

        attribs[std::string(fc+start,end-start)]=std::string(fc+start2,position-start2);
        position++;
    }

    for (;;)
    {
        while (iswhitespace(fc[position]))
        {
            position++;
        }

        start=position;
        while(fc[position]!='<')
        {
            if(fc[position]==0) throw(1);
            position++;
        }
        val=std::string(fc+start,position-start);

        if (fc[position+1]=='/')
        {
            position+=2;
            start=position;
            while(fc[position]!='>')
            {
                if(fc[position]==0) throw(1);
                position++;
            }

            if(name!=std::string(fc+start,position-start)) throw (1);

            position++;
            break;
        }
        else
        {
            XML c;
            c.read(file,position);
            children[childcount]=c;
            childcount++;
        }
    }
}

// read a complete xml file

XML XML::readfile(File &file)
{
    XML xml;
    int position=0;
    char *fc=(char*)file.contents;

    for(;;)
    {
        while (iswhitespace(fc[position]))
        {
            position++;
        }
        if (fc[position]!='<') throw(1);
        if (   fc[position+1]!='?'
            && fc[position+1]!='!') break;

        while (fc[position]!='>')
        {
            if(fc[position]==0) throw(1);
            position++;
        }
        position++;
    }

    xml.read(file,position);

    return xml;
}
