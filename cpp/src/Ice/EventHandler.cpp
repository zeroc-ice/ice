// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/EventHandler.h>


#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(EventHandler* p) { p->__incRef(); }
void IceInternal::decRef(EventHandler* p) { p->__decRef(); }

IceInternal::EventHandler::EventHandler(const InstancePtr& instance) :
    _instance(instance),
    _stream(instance)
{
}

IceInternal::EventHandler::~EventHandler()
{
}
