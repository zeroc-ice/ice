// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_DYNAMIC_LIBRARY_F_H
#define ICE_DYNAMIC_LIBRARY_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class DynamicLibrary;
ICE_API IceUtil::Shared* upCast(DynamicLibrary*);
typedef Handle<DynamicLibrary> DynamicLibraryPtr;

class DynamicLibraryList;
ICE_API IceUtil::Shared* upCast(DynamicLibraryList*);
typedef Handle<DynamicLibraryList> DynamicLibraryListPtr;

}
#endif
