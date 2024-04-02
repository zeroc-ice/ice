//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "Ice/Ice.h"

using namespace Test;

TestI::TestI() {}

void
TestI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
