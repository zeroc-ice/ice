// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class RemoteCommunicatorI : public Test::RemoteCommunicator
{
public:

    virtual Test::RemoteObjectAdapterPrx createObjectAdapter(int, int, int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

class RemoteObjectAdapterI : public Test::RemoteObjectAdapter
{
public:

    RemoteObjectAdapterI(const Ice::ObjectAdapterPtr&);

    virtual Test::TestIntfPrx getTestIntf(const Ice::Current&);
    virtual void activate(const Ice::Current&);
    virtual void hold(const Ice::Current&);
    virtual void deactivate(const Ice::Current&);

private:

    const Ice::ObjectAdapterPtr _adapter;
    const Test::TestIntfPrx _testIntf;
};

class TestI : public Test::TestIntf, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    virtual void sleep(int, const Ice::Current&);
    virtual void sleepAndHold(int, const Ice::Current&);
    virtual void interruptSleep(const Ice::Current&);
    virtual void waitForHeartbeat(int, const Ice::Current&);
};

#endif
