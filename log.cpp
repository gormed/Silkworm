
/*

    log.cpp

    write output into a log file


*/



#include <stdio.h>
#include <string.h>

#include "project.h"
#include "log.h"

std::ofstream &Log::log() { static std::ofstream _log; return _log; }

void Log::init()
{
    static const char* logHeader =

        "<html><head><title>" ENGINE " debug log</title>"
        "<style>html,body{font-family:Arial;font-size:11px;}</style>"
        "<body><h1>" ENGINE " debug log</h1>\n";

    log().open("log.html");

    log() << logHeader;
}

void Log::deinit()
{
    static const char*  logFooter = "</body></html>";

    log() << logFooter;

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

    log().flush();
}
