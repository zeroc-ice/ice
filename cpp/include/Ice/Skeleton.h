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

namespace IceServant { namespace Ice
{

class ICE_API ServantS : virtual public ::_Ice::SimpleShared
{
public:

protected:

    ServantS();
    virtual ~ServantS();

private:

    ServantS(const ServantS&);
    void operator=(const ServantS&);
};

} }

#endif
