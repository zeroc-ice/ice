// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_F_H
#define ICEE_OBJECT_F_H

#include <IceE/Handle.h>

namespace IceE
{

class Object;

}

namespace IceEInternal
{

ICEE_API void incRef(::IceE::Object*);
ICEE_API void decRef(::IceE::Object*);

}

namespace IceE
{

typedef IceEInternal::Handle< Object > ObjectPtr;

}

#endif
