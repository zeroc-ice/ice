// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONNECTION_F_H
#define ICEE_CONNECTION_F_H

#include <IceE/Handle.h>

namespace Ice
{

class Connection;
ICEE_API bool operator==(const Connection&, const Connection&);
ICEE_API bool operator!=(const Connection&, const Connection&);
ICEE_API bool operator<(const Connection&, const Connection&);

}

namespace IceInternal
{

ICEE_API void incRef(::Ice::Connection*);
ICEE_API void decRef(::Ice::Connection*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::Connection> ConnectionPtr;

}

#endif
