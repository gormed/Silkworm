
#ifndef FILE_H
#define FILE_H

/*

    file.h

    read files

*/

#include <stdio.h>

class File
{
    public:

    int length;
    unsigned char *contents;

    File()
    {
        contents=NULL;
        length=0;
    }

    ~File()
    {
       if(contents) delete []contents;
    }

    static File read(const char *filename)
    {
        File file;

        FILE *f;

        f = fopen(filename, "rb");

        if (!f) return file;

        fseek(f,0,SEEK_END);
        file.length = ftell(f);
        rewind(f);

        file.contents = new unsigned char[file.length];
        fread(file.contents,file.length,1,f);

        fclose(f);

        return file;
    }

    static File readText(const char *filename)
    {
        File file;

        FILE *f;

        f = fopen(filename, "rb");

        fseek(f,0,SEEK_END);
        file.length = ftell(f);
        rewind(f);

        file.contents = new unsigned char[file.length+1];
        fread(file.contents,file.length,1,f);
        file.contents[file.length+1]=0;

        fclose(f);

        return file;
    }

};

#endif

