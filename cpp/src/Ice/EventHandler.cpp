// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(EventHandler* p) { return p; }
#endif

IceInternal::EventHandler::EventHandler() :
#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
    _pending(SocketOperationNone),
    _started(SocketOperationNone),
    _completed(SocketOperationNone),
    _finish(false),
#else
    _disabled(SocketOperationNone),
#endif
    _ready(SocketOperationNone),
    _registered(SocketOperationNone)
{
}

IceInternal::EventHandler::~EventHandler()
{
}
