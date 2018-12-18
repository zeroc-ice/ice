// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_LOCAL_OBJECT_F_H
#define ICE_LOCAL_OBJECT_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace Ice
{

class LocalObject;
ICE_API IceUtil::Shared* upCast(::Ice::LocalObject*);
typedef IceInternal::Handle< LocalObject > LocalObjectPtr;

}

#endif
