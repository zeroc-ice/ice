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

namespace __Ice
{

void ICE_API incRef(::IceServant::Ice::ObjectS*);
void ICE_API decRef(::IceServant::Ice::ObjectS*);

}

namespace IceServant { namespace Ice
{

typedef __Ice::Handle< ObjectS > Object;

} }

#endif
