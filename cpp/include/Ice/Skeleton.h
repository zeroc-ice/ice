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

#include <Ice/Shared.h>
#include <Ice/Handle.h>

namespace Ice
{

class Servant;
void _incRef(Servant*);
void _decRef(Servant*);
typedef Handle<Servant> ServantHandle;

// ----------------------------------------------------------------------
// Servant
// ----------------------------------------------------------------------

class ICE_API Servant : virtual public SimpleShared
{
public:

protected:

    Servant();

private:

    Servant(const Servant&);
    void operator=(const Servant&);
};

}

#endif
