
/*

    resource.h

    base class for the Array, State and Image resources

*/

#ifndef RESOURCE_H
#define RESOURCE_H

#include <map>
#include <string>

#include "file.h"

// all resources share this base class in order to simplify implementing a resource manager

// resources generally are used in similar ways

// (1) loading

// (2) linking / compiling

// (3) delete data in system memory

// (4) use the resource

// (5) cleanup

class Resource
{
    public:

    virtual void link () = 0;
    virtual void unlink () = 0;

};


#endif




