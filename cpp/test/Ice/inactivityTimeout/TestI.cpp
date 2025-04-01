// Copyright (c) ZeroC, Inc.

#include "TestI.h"

#include <chrono>
#include <thread>

using namespace std;

void
TestIntfI::sleepAsync(int32_t ms, function<void()> response, function<void(exception_ptr)>, const Ice::Current&)
{
    _sleepFuture = std::async(
        std::launch::async,
        [ms, response]
        {
            this_thread::sleep_for(chrono::milliseconds(ms));
            response();
        });
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
