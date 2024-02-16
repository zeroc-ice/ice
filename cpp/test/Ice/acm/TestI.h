//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <mutex>
#include <condition_variable>

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

class TestI : public Test::TestIntf
{
public:

    virtual void sleep(int, const Ice::Current&);
    virtual void sleepAndHold(int, const Ice::Current&);
    virtual void interruptSleep(const Ice::Current&);
    virtual void startHeartbeatCount(const Ice::Current&);
    virtual void waitForHeartbeatCount(int, const Ice::Current&);

private:

    class HeartbeatCallbackI final : public std::enable_shared_from_this<HeartbeatCallbackI>
    {
    public:

        HeartbeatCallbackI() :
            _count(0)
        {
        }

        void
        waitForCount(int count)
        {
            std::unique_lock lock(_mutex);
            _condition.wait(lock, [this, count] { return _count >= count; });
        }

        virtual void
        heartbeat(const Ice::ConnectionPtr&)
        {
            std::lock_guard lock(_mutex);
            ++_count;
            _condition.notify_all();
        }

    private:

        int _count;
        std::mutex _mutex;
        std::condition_variable _condition;
    };
    using HeartbeatCallbackIPtr = std::shared_ptr<HeartbeatCallbackI>;

    HeartbeatCallbackIPtr _callback;
    std::mutex _mutex;
    std::condition_variable _condition;
};

#endif
