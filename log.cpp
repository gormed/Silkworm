
/*

    log.cpp

    write output into a log file


*/



#include <stdio.h>
#include <string.h>

#include "log.h"

std::ofstream glog;

std::ofstream &Log::log() { return glog; }

void Log::init()
{
    log().open("log.html");
}

void Log::deinit()
{
    log().close();
}

void Log::code(const char *text)
{
    static const char *html[] = { "&quot;", "&amp;", "&lt;"  "&gt;" };

    int length = strlen(text);
    int i,ln=1;
    int htmlent;

    log().width(4);

    log() << "<pre><i>1 </i>";

    for (i=0;i<length;i++)
    {
        htmlent=-1;
        switch(text[i])
        {
            case '"': htmlent=0; break;
            case '&': htmlent=1; break;
            case '<': htmlent=2; break;
            case '>': htmlent=3; break;
            case '\n': htmlent=-2; break;
            default: htmlent=-1;
        }
        if(htmlent==-1)
        {
            log() << text[i];
        }
        else if(htmlent==-2)
        {
            ln++;
            log() << "<br /><i>" << ln << "</i> ";
        }
        else
        {
            log() << html[htmlent];
        }
    }

    log() << "</pre>";
}
