//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <TestI.h>
#include <Ice/Ice.h>
#include <Dispatcher.h>
#include <TestHelper.h>

#include <thread>
#include <chrono>

using namespace std;

void
TestIntfI::op(const Ice::Current&)
{
    test(Dispatcher::isDispatcherThread());
}

void
TestIntfI::sleep(int32_t to, const Ice::Current&)
{
    this_thread::sleep_for(chrono::milliseconds(to));
}

void
TestIntfI::opWithPayload(Ice::ByteSeq, const Ice::Current&)
{
    test(Dispatcher::isDispatcherThread());
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    test(Dispatcher::isDispatcherThread());
    current.adapter->getCommunicator()->shutdown();
}

void
TestIntfControllerI::holdAdapter(const Ice::Current&)
{
    test(Dispatcher::isDispatcherThread());
    _adapter->hold();
}

void
TestIntfControllerI::resumeAdapter(const Ice::Current&)
{
    test(Dispatcher::isDispatcherThread());
    _adapter->activate();
}

TestIntfControllerI::TestIntfControllerI(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter) {}
