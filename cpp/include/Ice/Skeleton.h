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

namespace IceServant
{

class ICE_API ObjectS : virtual public ::_Ice::Shared
{
public:

protected:

    ObjectS();
    virtual ~ObjectS();

private:

    ObjectS(const ObjectS&);
    void operator=(const ObjectS&);
};

}

#endif
