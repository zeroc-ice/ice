// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;

void
TestIntfI::sleep(int32_t ms, const Ice::Current&)
{
    this_thread::sleep_for(chrono::milliseconds(ms));
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
TestIntfBidirI::makeSleep(
    bool aborted,
    int32_t ms,
    std::optional<Test::DelayedTestIntfPrx> prx,
    const Ice::Current& current)
{
    // Call asynchronously to avoid blocking the server thread
    prx->ice_fixed(current.con)
        ->sleepAsync(
            ms,
            [aborted]() { test(!aborted); },
            [aborted](const exception_ptr ex)
            {
                try
                {
                    rethrow_exception(ex);
                }
                catch (const Ice::ConnectionLostException&)
                {
                    test(aborted);
                }
                catch (...)
                {
                    test(false);
                }
            });
}
