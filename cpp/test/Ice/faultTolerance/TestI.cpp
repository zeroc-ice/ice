// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
