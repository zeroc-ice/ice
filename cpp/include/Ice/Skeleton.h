// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SKELETON_H
#define ICE_SKELETON_H

#include <Ice/SkeletonF.h>
#include <Ice/Shared.h>
#include <Ice/Incoming.h>

namespace IceServant { namespace Ice
{

class ICE_API ObjectS : virtual public ::__Ice::Shared
{
public:

    ObjectS();
    virtual ~ObjectS();

    virtual void __dispatch(::__Ice::Incoming*, const std::string&) = 0;

private:

    ObjectS(const ObjectS&);
    void operator=(const ObjectS&);
};

} }

#endif
