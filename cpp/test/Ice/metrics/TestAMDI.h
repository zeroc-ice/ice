// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "TestAMD.h"

class MetricsI final : public Test::Metrics
{
public:
    void opAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void failAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void
    opWithUserExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;

    void opWithRequestFailedExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void opWithLocalExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        final;

    void opWithUnknownExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) final;

    void
    opByteSAsync(Test::ByteSeq, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        final;
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
