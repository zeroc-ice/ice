// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    void sleepAsync(
        std::int32_t ms,
        std::function<void()> response,
        std::function<void(std::exception_ptr)> exception,
        const Ice::Current& current) final;

    void shutdown(const Ice::Current&) final;

private:
    // has a blocking destructor
    std::future<void> _sleepFuture;
};
#endif
