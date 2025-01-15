//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"

using namespace std;

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
