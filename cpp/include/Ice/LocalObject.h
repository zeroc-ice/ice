// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_LOCAL_OBJECT_H
#define ICE_LOCAL_OBJECT_H

#include <IceUtil/Shared.h>
#include <Ice/LocalObjectF.h>

namespace IceInternal
{

class BasicStream;

}

namespace Ice
{

class ICE_API LocalObject : virtual public ::IceUtil::Shared
{
public:

    virtual bool operator==(const LocalObject&) const;
    virtual bool operator!=(const LocalObject&) const;
    virtual bool operator<(const LocalObject&) const;
    virtual ::Ice::Int ice_hash() const;
};

}

#endif
