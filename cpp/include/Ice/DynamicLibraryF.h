// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
