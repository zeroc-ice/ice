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
TestIntfI::setCallback(std::optional<Test::TestIntfPrx> prx, const Ice::Current& current)
{
    // Call from a detach thread to avoid blocking the server thread
    std::thread(
        [prx, current]()
        {
            try
            {
                prx->ice_fixed(current.con)->sleep(2000);
            }
            catch (const Ice::ConnectionLostException&)
            {
            }
        })
        .detach();
}
