// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
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
    _stream(_instance) // Must be _instance, and not instance, because BasicStream does not duplicate the instance!
{
}

IceInternal::EventHandler::~EventHandler()
{
}
