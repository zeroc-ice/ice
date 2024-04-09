//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

void
testIdleCheckWithExhaustedThreadPool(const TestIntfPrx& p)
{
    cout << "testing idle timeout with exhausted OA thread pool... " << flush;
    p->ice_ping();
    ConnectionPtr connection = p->ice_getCachedConnection();
    test(connection);
    p->sleep(2250);
    test(p->ice_getCachedConnection() == connection); // we still have the same connection
    cout << "ok" << endl;
}

void
testMismatchedIdleTimeout(const string& proxyString, const PropertiesPtr& properties)
{
    cout << "testing mismatched idle timeout... " << flush;

    // Create a new communicator with the desired properties.
    Ice::InitializationData initData;
    initData.properties = properties->clone();
    initData.properties->setProperty("Ice.IdleTimeout", "3");
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    Ice::CommunicatorHolder holder = initialize(initData);
    TestIntfPrx p(holder.communicator(), proxyString);

    p->ice_ping();
    ConnectionPtr connection = p->ice_getCachedConnection();
    test(connection);

    // The idle check on the server side aborts the connection because it doesn't get a heartbeat in a timely fashion.
    try
    {
        p->sleep(2250);
        test(false); // the server aborts the connection after about 1 second.
    }
    catch (const ConnectionLostException&)
    {
        // Expected
    }
    cout << "ok" << endl;
}

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    string proxyString = "test: " + helper->getTestEndpoint();
    TestIntfPrx p(communicator, proxyString);

    testIdleCheckWithExhaustedThreadPool(p);
    testMismatchedIdleTimeout(proxyString, communicator->getProperties());

    p->shutdown();
}
