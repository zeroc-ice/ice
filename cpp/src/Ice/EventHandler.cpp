// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EventHandler.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(EventHandler* p) { return p; }

IceInternal::EventHandler::EventHandler() :
#ifdef ICE_USE_IOCP
    _pending(SocketOperationNone),
    _ready(SocketOperationNone),
    _started(SocketOperationNone),
    _finish(false),
#else
    _disabled(SocketOperationNone),
#endif
    _registered(SocketOperationNone)
{
}

IceInternal::EventHandler::~EventHandler()
{
}
