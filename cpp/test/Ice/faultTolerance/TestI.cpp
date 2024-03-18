//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

#ifndef _WIN32
#    include <unistd.h>
#endif

TestI::TestI() {}

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

int32_t
TestI::pid(const Ice::Current&)
{
#ifdef _MSC_VER
    return _getpid();
#else
    return getpid();
#endif
}
