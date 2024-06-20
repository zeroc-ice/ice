//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
TestIntfBidirI::putAsSleep(
    bool aborted,
    int32_t ms,
    std::optional<Test::DelayedTestIntfPrx> prx,
    const Ice::Current& current)
{
    // Call from a detach thread to avoid blocking the server thread
    std::thread(
        [prx, ms, current, aborted]()
        {
            try
            {
                prx->ice_fixed(current.con)->sleep(ms);
                test(!aborted);
            }
            catch (const Ice::ConnectionLostException&)
            {
                test(aborted);
            }
        })
        .detach();
}
