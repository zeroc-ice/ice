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

    /*
    cout << "testing idle timeout heartbeats... " << flush;
    int heartbeatCount = 0; // the heartbeats we receive from the server.

    p->init();

    // Since the connection is fully established at this point, we don't count the initial ValidateConnection heartbeat.
    p->ice_getCachedConnection()->setHeartbeatCallback([&heartbeatCount](const ConnectionPtr&) { ++heartbeatCount; });

    this_thread::sleep_for(chrono::milliseconds(3250));
    test(p->getHeartbeatCount() == 2);
    test(heartbeatCount == 6);

    // Verifies writes (the getHeartbeatCount round-trip) skips a heartbeat.
    this_thread::sleep_for(chrono::milliseconds(1250));
    test(p->getHeartbeatCount() == 2);
    test(heartbeatCount == 8);
    cout << "ok" << endl;
    */

   p->ice_getConnection();

   //p->ice_ping();
   cerr << "sleeping for 5 seconds..." << endl;
   this_thread::sleep_for(chrono::milliseconds(5000));

    p->shutdown();
}
