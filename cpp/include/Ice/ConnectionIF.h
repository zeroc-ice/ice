// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTIONI_F_H
#define ICE_CONNECTIONI_F_H

#include <Ice/Handle.h>
#include <Ice/LocalObject.h>

namespace Ice
{

class ConnectionI;
ICE_API Ice::LocalObject* upCast(Ice::ConnectionI*);
typedef IceInternal::Handle<ConnectionI> ConnectionIPtr;

}

namespace IceInternal
{

enum AsyncStatus
{
    AsyncStatusQueued = 0,
    AsyncStatusSent = 1,
    AsyncStatusInvokeSentCallback = 2
};

}

#endif
