//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;

void
TestIntfI::init(const Ice::Current& current)
{
    _heartbeatCount = 0;
    current.con->setHeartbeatCallback([this](const Ice::ConnectionPtr&) { this->incrementHeartbeatCount(); });
}

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
