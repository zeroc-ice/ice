// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_BUFFER_H
#define ICE_BUFFER_H

#include <Ice/Config.h>

namespace _Ice
{

class ICE_API Buffer
{
public:

    Buffer() : i(b.begin()) { }

    typedef std::vector<Ice::Byte> Container;
    Container b;
    Container::iterator i;

private:

    Buffer(const Buffer&);
    void operator=(const Buffer&);
};

}

#endif
