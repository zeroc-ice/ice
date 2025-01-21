// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Ice/Ice.h"
#include "Test.h"

class TestIntfControllerI;
using TestIntfControllerIPtr = std::shared_ptr<TestIntfControllerI>;

class TestIntfI : public virtual Test::TestIntf
{
public:
    void op(const Ice::Current&) override;
    void sleep(std::int32_t, const Ice::Current&) override;
    void opWithPayload(Ice::ByteSeq, const Ice::Current&) override;
    void shutdown(const Ice::Current&) override;
};

class TestIntfControllerI : public Test::TestIntfController
{
public:
    void holdAdapter(const Ice::Current&) override;
    void resumeAdapter(const Ice::Current&) override;

    TestIntfControllerI(Ice::ObjectAdapterPtr);

private:
    Ice::ObjectAdapterPtr _adapter;
};

#endif
