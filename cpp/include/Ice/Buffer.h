// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_BUFFER_H
#define ICE_BUFFER_H

#include <Ice/Config.h>

namespace IceInternal
{

class ICE_API Buffer : public ::IceUtil::noncopyable
{
public:

    // TODO: Should not be inline, as this is not performance critical.
    Buffer()
    {
	b.reserve(1000);
	i = b.begin();
    }

    typedef std::vector<Ice::Byte> Container;
    Container b;
    Container::iterator i;
};

}

#endif
