// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_ADAPTER_F_H
#define ICEE_OBJECT_ADAPTER_F_H

#include <IceE/Handle.h>

namespace IceE
{

class ObjectAdapter;
ICEE_API bool operator==(const ObjectAdapter&, const ObjectAdapter&);
ICEE_API bool operator!=(const ObjectAdapter&, const ObjectAdapter&);
ICEE_API bool operator<(const ObjectAdapter&, const ObjectAdapter&);

}

namespace IceEInternal
{

ICEE_API void incRef(::IceE::ObjectAdapter*);
ICEE_API void decRef(::IceE::ObjectAdapter*);

}

namespace IceE
{

typedef ::IceEInternal::Handle< ::IceE::ObjectAdapter> ObjectAdapterPtr;

}

#endif
