// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_REFERENCE_F_H
#define ICE_REFERENCE_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Reference;
ICE_API void incRef(Reference*);
ICE_API void decRef(Reference*);
typedef IceInternal::Handle<Reference> ReferencePtr;

}

#endif
