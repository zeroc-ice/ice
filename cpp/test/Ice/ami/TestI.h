// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfControllerI;
using TestIntfControllerIPtr = std::shared_ptr<TestIntfControllerI>;

class TestIntfI final : public Test::TestIntf
{
public:
    TestIntfI();

    void op(const Ice::Current&) final;
    int opWithResult(const Ice::Current&) final;
    void opWithUE(const Ice::Current&) final;
    int opWithResultAndUE(const Ice::Current&) final;
    void opWithPayload(Ice::ByteSeq, const Ice::Current&) final;
    void opBatch(const Ice::Current&) final;
    std::int32_t opBatchCount(const Ice::Current&) final;
    void opWithArgs(
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        std::int32_t&,
        const Ice::Current&) final;
    bool waitForBatch(std::int32_t, const Ice::Current&) final;
    void closeConnection(const Ice::Current&) final;
    void abortConnection(const Ice::Current&) final;
    void sleep(std::int32_t, const Ice::Current&) final;
    void startDispatchAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;
    void finishDispatch(const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

    bool supportsFunctionalTests(const Ice::Current&) final;
    bool supportsBackPressureTests(const Ice::Current&) final;

    void pingBiDirAsync(
        std::optional<Test::PingReplyPrx>,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

private:
    int _batchCount{0};
    bool _shutdown{false};
    std::function<void()> _pending;
    std::mutex _mutex;
    std::condition_variable _condition;
};

class TestIntfControllerI : public Test::TestIntfController
{
public:
    void holdAdapter(const Ice::Current&) final;
    void resumeAdapter(const Ice::Current&) final;

    TestIntfControllerI(Ice::ObjectAdapterPtr);

private:
    Ice::ObjectAdapterPtr _adapter;
    std::mutex _mutex;
};

class TestIntfII : public Test::Outer::Inner::TestIntf
{
public:
    std::int32_t op(std::int32_t, std::int32_t&, const Ice::Current&) final;
};

#endif
