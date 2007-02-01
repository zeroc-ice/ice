// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    if(seconds <= 0)
    {
        current.adapter->hold();
        current.adapter->activate();
    }
    else
    {
        assert(false); // TODO
    }
}

void
HoldI::shutdown(const Ice::Current& current)
{
    current.adapter->hold();
    current.adapter->getCommunicator()->shutdown();
}
