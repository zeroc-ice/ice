// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class MetricsI final : public Test::Metrics
{
    void op(const Ice::Current&) final;

    void fail(const Ice::Current&) final;

    void opWithUserException(const Ice::Current&) final;

    void opWithRequestFailedException(const Ice::Current&) final;

    void opWithLocalException(const Ice::Current&) final;

    void opWithUnknownException(const Ice::Current&) final;

    void opByteS(Test::ByteSeq, const Ice::Current&) final;

    std::optional<Ice::ObjectPrx> getAdmin(const Ice::Current&) final;

    void shutdown(const Ice::Current&) final;
};

class ControllerI final : public Test::Controller
{
public:
    ControllerI(Ice::ObjectAdapterPtr);

    void hold(const Ice::Current&) final;

    void resume(const Ice::Current&) final;

private:
    const Ice::ObjectAdapterPtr _adapter;
};

#endif
