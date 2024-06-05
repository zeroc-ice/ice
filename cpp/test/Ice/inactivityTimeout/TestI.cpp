//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "Ice/Ice.h"

#include <chrono>

using namespace std;

void
TestIntfI::sleep(int32_t ms, const Ice::Current&)
{
    this_thread::sleep_for(chrono::milliseconds(ms));
}

void
TestIntfI::sendPayload(Ice::ByteSeq, const Ice::Current&)
{
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
TestIntfControllerI::holdAdapter(const Ice::Current&)
{
    _adapter->hold();
}

void
TestIntfControllerI::resumeAdapter(const Ice::Current&)
{
    _adapter->activate();
}

TestIntfControllerI::TestIntfControllerI(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter) {}
