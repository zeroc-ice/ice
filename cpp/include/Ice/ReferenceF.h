// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_REFERENCE_F_H
#define ICE_REFERENCE_F_H

#include <Ice/Handle.h>

namespace _Ice
{

class ReferenceI;
void ICE_API _incRef(ReferenceI*);
void ICE_API _decRef(ReferenceI*);
typedef _Ice::Handle<ReferenceI> Reference;

}

#endif
