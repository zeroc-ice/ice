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

namespace IceServant { namespace Ice { class ObjectS; } }

namespace _Ice
{

void ICE_API _incRef(::IceServant::Ice::ObjectS*);
void ICE_API _decRef(::IceServant::Ice::ObjectS*);

}

namespace IceServant { namespace Ice
{

typedef _Ice::Handle< ObjectS > Object;

} }

#endif
