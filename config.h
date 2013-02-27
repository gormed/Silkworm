
#ifndef CONFIG_H
#define CONFIG_H

#include "xml.h"

class Config
{
    private:

        static XML &xml()
        {
            static XML _xml;
            return _xml;
        }


    public:

        static void read()
        {
            File configFile = File::read("config.xml");
            xml() = XML::readfile(configFile);
        }

        static std::string value ( std::string name ) {
            return xml().get(name)[0]["value"];
        }

};



#endif

