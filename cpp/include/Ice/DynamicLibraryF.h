// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_DYNAMIC_LIBRARY_F_H
#define ICE_DYNAMIC_LIBRARY_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class DynamicLibrary;
ICE_API void incRef(DynamicLibrary*);
ICE_API void decRef(DynamicLibrary*);
typedef Handle<DynamicLibrary> DynamicLibraryPtr;

}

#endif
