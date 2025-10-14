// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class MetricsI final : public Test::AsyncMetrics
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

    void getAdminAsync(
        std::function<void(const std::optional<Ice::ObjectPrx>&)> response,
        std::function<void(std::exception_ptr)> error,
        const Ice::Current&) final;

    void shutdownAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;
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
