// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_F_H
#define ICE_CONNECTION_F_H

#include <IceE/Handle.h>

namespace Ice
{

class Connection;
ICE_API bool operator==(const Connection&, const Connection&);
ICE_API bool operator!=(const Connection&, const Connection&);
ICE_API bool operator<(const Connection&, const Connection&);

}

namespace IceInternal
{

ICE_API void incRef(::Ice::Connection*);
ICE_API void decRef(::Ice::Connection*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::Connection> ConnectionPtr;

}

#endif
