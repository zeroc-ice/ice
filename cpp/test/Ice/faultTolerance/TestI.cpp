// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

#ifndef _WIN32
#    include <unistd.h>
#endif

TestI::TestI() = default;

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
