// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TimeoutI final : public Test::Timeout
{
public:
    void op(const Ice::Current&) final;
    void sendData(Test::ByteSeq, const Ice::Current&) final;
    void sleep(std::int32_t, const Ice::Current&) final;
};

class ControllerI final : public Test::Controller
{
public:
    ControllerI(Ice::ObjectAdapterPtr);

    void holdAdapter(std::int32_t, const Ice::Current&) final;
    void resumeAdapter(const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

private:
    const Ice::ObjectAdapterPtr _adapter;
};

#endif
