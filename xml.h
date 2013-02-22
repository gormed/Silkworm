
/*

    xml.h

    class for loading xml files and accessing the xml data structure

*/

#ifndef XML_H
#define XML_H

#include <map>
#include <string>

#include "file.h"

class XML;

typedef std::map<std::string, std::string> AttribMap;
typedef std::map < int , XML > NodeMap;

// the xml class contains a nested tag structure

class XML
{
    private:

    std::string name;
    std::string val;

    AttribMap attribs;
    NodeMap children;

    public:

    void read(File &file, int &position);

    // use the bracket operator to access an xml tags attributes

    const std::string& operator[] ( std::string index) { return attribs[index]; }

    // use get() to retrieve a map of nested tags, if any

    NodeMap get() const { return children; }

    // alternative get() for prefiltering only nested tags with a certain name

    NodeMap get(std::string filter)
    {
        NodeMap m; int i=0;
        NodeMap::iterator p;
		for(p = children.begin(); p!=children.end(); ++p)
		{
			if (p->second.tag()==filter)
			{
			    m[i]=p->second;
			    i++;
			}
		}
		return m;
    }

    // use value() to get a tags content

    const std::string& value() const { return val; }

    // use tag() to get a tags name

    const std::string& tag() const { return name; }


    // static method for reading a whole xml file
    // the object returned will be the root xml tag

    static XML readfile(File &file);


};

#endif

