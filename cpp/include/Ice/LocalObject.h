// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

class ICE_API LocalObject : public ::IceUtil::Shared
{
public:

    bool operator==(const LocalObject&) const;
    bool operator!=(const LocalObject&) const;
    bool operator<(const LocalObject&) const;
    ::Ice::Int ice_hash() const;
};

}

#endif
