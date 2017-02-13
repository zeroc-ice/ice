// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>
#include <Dispatcher.h>
#include <TestCommon.h>
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
TestIntfI::opWithPayload(const Ice::ByteSeq&, const Ice::Current&)
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
