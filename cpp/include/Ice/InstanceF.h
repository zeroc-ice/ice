// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INSTANCE_F_H
#define ICE_INSTANCE_F_H

#include <Ice/Handle.h>

namespace _Ice
{

class InstanceI;
void ICE_API _incRef(InstanceI*);
void ICE_API _decRef(InstanceI*);
typedef _Ice::Handle<InstanceI> Instance;

}

#endif
