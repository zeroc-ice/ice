//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <TestHelper.h>

class TestIntfControllerI;
using TestIntfControllerIPtr = std::shared_ptr<TestIntfControllerI>;

class TestIntfI : public virtual Test::TestIntf
{
public:
    TestIntfI();

    virtual void op(const Ice::Current&);
    virtual int opWithResult(const Ice::Current&);
    virtual void opWithUE(const Ice::Current&);
    virtual int opWithResultAndUE(const Ice::Current&);
    virtual void opWithPayload(Ice::ByteSeq, const Ice::Current&);
    virtual void opBatch(const Ice::Current&);
    virtual std::int32_t opBatchCount(const Ice::Current&);
    virtual void opWithArgs(
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
        const Ice::Current&);
    virtual bool waitForBatch(std::int32_t, const Ice::Current&);
    virtual void close(Test::CloseMode, const Ice::Current&);
    virtual void sleep(std::int32_t, const Ice::Current&);
    virtual void
    startDispatchAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&);
    virtual void finishDispatch(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    virtual bool supportsAMD(const Ice::Current&);
    virtual bool supportsFunctionalTests(const Ice::Current&);

    virtual void pingBiDir(Test::PingReplyPrxPtr, const Ice::Current&);

private:
    int _batchCount;
    bool _shutdown;
    std::function<void()> _pending;
    std::mutex _mutex;
    std::condition_variable _condition;
};

class TestIntfControllerI : public Test::TestIntfController
{
public:
    virtual void holdAdapter(const Ice::Current&);
    virtual void resumeAdapter(const Ice::Current&);

    TestIntfControllerI(const Ice::ObjectAdapterPtr&);

private:
    Ice::ObjectAdapterPtr _adapter;
    std::mutex _mutex;
};

class TestIntfII : public virtual Test::Outer::Inner::TestIntf
{
public:
    std::int32_t op(std::int32_t, std::int32_t&, const Ice::Current&);
};

#endif
