// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    _stream(_instance.get())
{
}

IceInternal::EventHandler::~EventHandler()
{
}
