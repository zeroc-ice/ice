// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

InstancePtr
IceInternal::EventHandler::instance() const
{
    return _instance;
}

IceInternal::EventHandler::EventHandler(const InstancePtr& instance) :
    _instance(instance),
    _stream(_instance.get())
{
}

IceInternal::EventHandler::~EventHandler()
{
}
