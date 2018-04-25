// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

TestI::TestI()
{
}

void
TestI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
TestI::abort(const Ice::Current&)
{
    _exit(0);
}

void
TestI::idempotentAbort(const Ice::Current&)
{
    _exit(0);
}

Ice::Int
TestI::pid(const Ice::Current&)
{
#ifdef _MSC_VER
    return _getpid();
#else
    return getpid();
#endif
}
