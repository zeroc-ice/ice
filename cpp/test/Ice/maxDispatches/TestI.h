//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    ~TestIntfI() override;

    void opAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;
    std::int32_t resetMaxConcurrentDispatches(const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

    void decDispatchCount();

private:
    Ice::Timer _timer;
    std::mutex _mutex;
    std::int32_t _dispatchCount = 0;
    std::int32_t _maxDispatchCount = 0;
};

#endif
