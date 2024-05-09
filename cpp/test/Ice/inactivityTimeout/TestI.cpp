//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"

#include <chrono>

using namespace std;

void
TestIntfI::sleep(int32_t ms, const Ice::Current&)
{
    this_thread::sleep_for(chrono::milliseconds(ms));
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
