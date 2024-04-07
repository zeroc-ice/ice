//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Test.h"
#include "TestHelper.h"

#include <atomic>

using namespace std;
using namespace Ice;
using namespace Test;

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();

    TestIntfPrx p(communicator, "test:" + helper->getTestEndpoint());
    std::atomic<int> heartbeatCount = 0; // the heartbeats we receive from the server.

    cout << "testing idle timeout with exhausted OA thread pool... " << flush;

    p->init();
    ConnectionPtr connection = p->ice_getConnection();
    test(connection);

    // Since the connection is fully established at this point, we don't count the initial ValidateConnection heartbeat.
    p->ice_getCachedConnection()->setHeartbeatCallback([&heartbeatCount](const ConnectionPtr&) { ++heartbeatCount; });

    p->sleep(2250);
    test(p->ice_getCachedConnection() == connection); // we still have the same connection
    test(p->getHeartbeatCount() == 4);
    test(heartbeatCount == 4);
    connection->close(ConnectionClose::GracefullyWithWait);
    cout << "ok" << endl;

    cout << "testing idle timeout heartbeats... " << flush;
    heartbeatCount = 0;
    p->init();
    p->ice_getCachedConnection()->setHeartbeatCallback([&heartbeatCount](const ConnectionPtr&) { ++heartbeatCount; });

    this_thread::sleep_for(chrono::milliseconds(1250));
    test(p->getHeartbeatCount() == 2);
    test(heartbeatCount == 2);

    // Verifies writes (the getHeartbeatCount round-trip) skips a heartbeat.
    this_thread::sleep_for(chrono::milliseconds(1400));
    test(p->getHeartbeatCount() == 4);
    test(heartbeatCount == 4);
    cout << "ok" << endl;

    p->shutdown();
}
