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

#include <Ice/LocalObjectF.h>
#include <Ice/Shared.h>

namespace Ice
{

// No virtual inheritance for local objects
class ICE_API LocalObject : public ::__Ice::Shared
{
public:

    LocalObject();
    virtual ~LocalObject();
};

}

#endif
