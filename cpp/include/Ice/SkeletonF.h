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

namespace IceServant { class ObjectS; }

namespace _Ice
{

void ICE_API _incRef(::IceServant::ObjectS*);
void ICE_API _decRef(::IceServant::ObjectS*);

}

namespace IceServant
{

typedef _Ice::Handle< ObjectS > Object;

}

#endif
