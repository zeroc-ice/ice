// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SKELETON_F_H
#define ICE_SKELETON_F_H

#include <Ice/Handle.h>

namespace IceServant { namespace Ice { class ServantS; } }

namespace _Ice
{

void ICE_API _incRef(::IceServant::Ice::ServantS*);
void ICE_API _decRef(::IceServant::Ice::ServantS*);

}

namespace Ice
{

typedef _Ice::Handle< ::IceServant::Ice::ServantS > Servant;

}

#endif
