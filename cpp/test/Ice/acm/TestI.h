// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class RemoteCommunicatorI : public Test::RemoteCommunicator
{
public:

    virtual Test::RemoteObjectAdapterPrxPtr createObjectAdapter(int, int, int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

class RemoteObjectAdapterI : public Test::RemoteObjectAdapter
{
public:

    RemoteObjectAdapterI(const Ice::ObjectAdapterPtr&);

    virtual Test::TestIntfPrxPtr getTestIntf(const Ice::Current&);
    virtual void activate(const Ice::Current&);
    virtual void hold(const Ice::Current&);
    virtual void deactivate(const Ice::Current&);

private:

    const Ice::ObjectAdapterPtr _adapter;
    const Test::TestIntfPrxPtr _testIntf;
};

class TestI : public Test::TestIntf, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    virtual void sleep(int, const Ice::Current&);
    virtual void sleepAndHold(int, const Ice::Current&);
    virtual void interruptSleep(const Ice::Current&);
    virtual void startHeartbeatCount(const Ice::Current&);
    virtual void waitForHeartbeatCount(int, const Ice::Current&);

private:

    class HeartbeatCallbackI ICE_FINAL :
#ifdef ICE_CPP11_MAPPING
                                public std::enable_shared_from_this<HeartbeatCallbackI>,
#else
                                public Ice::HeartbeatCallback,
#endif
                                private IceUtil::Monitor<IceUtil::Mutex>
    {
    public:

        HeartbeatCallbackI() :
            _count(0)
        {
        }

        void
        waitForCount(int count)
        {
            Lock sync(*this);
            while(_count < count)
            {
                wait();
            }
        }

        virtual void
        heartbeat(const Ice::ConnectionPtr&)
        {
            Lock sync(*this);
            ++_count;
            notifyAll();
        }

    private:

        int _count;
    };
    ICE_DEFINE_PTR(HeartbeatCallbackIPtr, HeartbeatCallbackI);

    HeartbeatCallbackIPtr _callback;
};

#endif
