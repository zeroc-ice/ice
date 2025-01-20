// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class ResponderI final : public Test::Responder
{
public:
    void start(const Ice::Current&) final;
    void stop(const Ice::Current&) final;
    std::int32_t pendingResponseCount(const Ice::Current&) final;

    void queueResponse(std::function<void()>);

private:
    std::mutex _mutex;
    std::vector<std::function<void()>> _responses;
    bool _started = false;
};

class TestIntfI final : public Test::TestIntf
{
public:
    void opAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) final;
    std::int32_t resetMaxConcurrentDispatches(const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;

    TestIntfI(std::shared_ptr<ResponderI> responder);

private:
    void decDispatchCount();

    const std::shared_ptr<ResponderI> _responder;

    std::mutex _mutex;
    std::int32_t _dispatchCount = 0;
    std::int32_t _maxDispatchCount = 0;
};

#endif
