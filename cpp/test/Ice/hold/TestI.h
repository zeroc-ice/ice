// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class HoldI final : public Test::Hold
{
public:
    HoldI(Ice::ObjectAdapterPtr);

    void putOnHoldAsync(
        std::int32_t delay,
        std::function<void()> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current&) final;

    void waitForHoldAsync(
        std::function<void()> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current&) final;

    std::int32_t set(std::int32_t, std::int32_t, const Ice::Current&) final;

    void setOneway(std::int32_t, std::int32_t, const Ice::Current&) final;

    void shutdown(const Ice::Current&) final;

private:
    int _last{0};
    const Ice::ObjectAdapterPtr _adapter;
    std::mutex _mutex;
    std::mutex _taskMutex;
};

#endif
