// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
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

class DynamicLibraryList;
ICE_API void incRef(DynamicLibraryList*);
ICE_API void decRef(DynamicLibraryList*);
typedef Handle<DynamicLibraryList> DynamicLibraryListPtr;

}

#endif
