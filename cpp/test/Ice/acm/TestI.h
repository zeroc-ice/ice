//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <mutex>
#include <condition_variable>

class RemoteCommunicatorI final : public Test::RemoteCommunicator
{
public:

    std::optional<Test::RemoteObjectAdapterPrx> createObjectAdapter(int, int, int, const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;
};

class RemoteObjectAdapterI final : public Test::RemoteObjectAdapter
{
public:

    RemoteObjectAdapterI(const Ice::ObjectAdapterPtr&);

    std::optional<Test::TestIntfPrx> getTestIntf(const Ice::Current&) final;
    void activate(const Ice::Current&) final;
    void hold(const Ice::Current&) final;
    void deactivate(const Ice::Current&) final;

private:

    const Ice::ObjectAdapterPtr _adapter;
    const Test::TestIntfPrxPtr _testIntf;
};

class TestI final : public Test::TestIntf
{
public:

    void sleep(int, const Ice::Current&) final;
    void sleepAndHold(int, const Ice::Current&) final;
    void interruptSleep(const Ice::Current&) final;
    void startHeartbeatCount(const Ice::Current&) final;
    void waitForHeartbeatCount(int, const Ice::Current&) final;

private:

    class HeartbeatCallbackI final : public std::enable_shared_from_this<HeartbeatCallbackI>
    {
    public:

        HeartbeatCallbackI() :
            _count(0)
        {
        }

        void waitForCount(int count)
        {
            std::unique_lock lock(_mutex);
            _condition.wait(lock, [this, count] { return _count >= count; });
        }

        void heartbeat(const Ice::ConnectionPtr&)
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
