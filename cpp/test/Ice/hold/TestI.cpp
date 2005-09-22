// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

void
HoldI::putOnHold(Ice::Int seconds, const Ice::Current& current)
{
    current.adapter->hold();
    current.adapter->activate();
}

void
HoldI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
