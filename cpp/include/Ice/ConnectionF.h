// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_F_H
#define ICE_CONNECTION_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Connection;
ICE_API void incRef(Connection*);
ICE_API void decRef(Connection*);
typedef IceInternal::Handle<Connection> ConnectionPtr;

}

#endif
