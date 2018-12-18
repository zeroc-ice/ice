// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>
#include <Dispatcher.h>
#include <TestHelper.h>
#include <IceUtil/Thread.h>

using namespace std;

void
TestIntfI::op(const Ice::Current&)
{
    test(Dispatcher::isDispatcherThread());
}

void
TestIntfI::sleep(Ice::Int to, const Ice::Current&)
{
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(to));
}

void
#ifdef ICE_CPP11_MAPPING
TestIntfI::opWithPayload(Ice::ByteSeq, const Ice::Current&)
#else
TestIntfI::opWithPayload(const Ice::ByteSeq&, const Ice::Current&)
#endif
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

TestIntfControllerI::TestIntfControllerI(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter)
{
}
