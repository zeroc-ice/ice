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

class Reference;
void incRef(Reference*);
void decRef(Reference*);
typedef __Ice::Handle<Reference> Reference_ptr;

}

#endif
