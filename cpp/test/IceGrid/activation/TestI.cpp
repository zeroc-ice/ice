// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace Test;

TestI::TestI() = default;

void
TestI::fail(const Ice::Current& current)
{
    _failed = true;
    current.adapter->getCommunicator()->shutdown();
}

void
TestI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

bool
TestI::isFailed() const
{
    return _failed;
}
