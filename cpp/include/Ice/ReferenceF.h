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

namespace __Ice
{

class ReferenceI;
void ICE_API incRef(ReferenceI*);
void ICE_API decRef(ReferenceI*);
typedef __Ice::Handle<ReferenceI> Reference;

}

#endif
