//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "TestI.h"
#include "TestHelper.h"

#include <thread>
#include <chrono>

using namespace std;
using namespace Ice;
using namespace Test;

void
TestI::transient(const Current& current)
{
    CommunicatorPtr communicator = current.adapter->getCommunicator();
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints(
        "TransientTestAdapter",
        TestHelper::getTestEndpoint(communicator->getProperties(), 1));
    adapter->activate();
    adapter->destroy();
}

void
TestI::deactivate(const Current& current)
{
    current.adapter->deactivate();
    this_thread::sleep_for(chrono::milliseconds(100));
}

string
Cookie::message() const
{
    return "blahblah";
}
