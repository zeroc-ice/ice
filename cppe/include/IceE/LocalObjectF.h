// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCAL_OBJECT_F_H
#define ICEE_LOCAL_OBJECT_F_H

#include <IceE/Handle.h>

namespace IceE
{

class LocalObject;

}

namespace IceEInternal
{

ICEE_API void incRef(::IceE::LocalObject*);
ICEE_API void decRef(::IceE::LocalObject*);

}

namespace IceE
{

typedef IceEInternal::Handle< LocalObject > LocalObjectPtr;

}

#endif
