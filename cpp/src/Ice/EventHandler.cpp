// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

void IceInternal::incRef(EventHandler* p) { p->__incRef(); }
void IceInternal::decRef(EventHandler* p) { p->__decRef(); }

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
