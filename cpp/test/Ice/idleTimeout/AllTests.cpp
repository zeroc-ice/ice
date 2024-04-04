//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();

    TestIntfPrx p(communicator, "test:" + helper->getTestEndpoint());
    cout << "testing idle timeout... " << flush;

    int heartbeatCount = 0; // the heartbeats we receive from the server.

    p->init();

    // Since the connection is fully established at this point, we don't count the initial ValidateConnection heartbeat.
    p->ice_getCachedConnection()->setHeartbeatCallback([&heartbeatCount](const ConnectionPtr&) { ++heartbeatCount; });

    this_thread::sleep_for(chrono::milliseconds(3500));
    test(p->getHeartbeatCount() == 3);
    test(heartbeatCount == 3);

    // Verifies writes (the getHeartbeat round-trip) skips a heartbeat.
    this_thread::sleep_for(chrono::milliseconds(1900));
    test(p->getHeartbeatCount() == 4);
    test(heartbeatCount == 4);
    cout << "ok" << endl;

    p->shutdown();
}
