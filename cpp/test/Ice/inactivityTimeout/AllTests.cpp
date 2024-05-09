//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

void
testClientInactivityTimeout(const TestIntfPrx& p)
{
    cout << "testing that the client side inactivity timeout shuts down the connection... " << flush;
    p->ice_ping();
    ConnectionPtr connection = p->ice_getCachedConnection();
    test(connection);

    // The inactivity timeout is 1s on the client side and 2s on the server side. 1.5 seconds tests the client side.
    this_thread::sleep_for(chrono::milliseconds(1500));
    p->ice_ping();
    ConnectionPtr connection2 = p->ice_getCachedConnection();
    test(connection2 != connection);
    cout << "ok" << endl;
}

void
testServerInactivityTimeout(const string& proxyString, const PropertiesPtr& properties)
{
    cout << "testing that the server side inactivity timeout shuts down the connection... " << flush;

    // Create a new communicator with the desired properties.
    Ice::InitializationData initData;
    initData.properties = properties->clone();
    initData.properties->setProperty("Ice.Connection.InactivityTimeout", "2");
    Ice::CommunicatorHolder holder = initialize(initData);
    TestIntfPrx p(holder.communicator(), proxyString);

    p->ice_ping();
    ConnectionPtr connection = p->ice_getCachedConnection();
    test(connection);

    // The inactivity timeout is 2s on the client side and 1s on the server side. 1.5 seconds tests the server side.
    this_thread::sleep_for(chrono::milliseconds(1500));
    p->ice_ping();
    ConnectionPtr connection2 = p->ice_getCachedConnection();
    test(connection2 != connection);
    cout << "ok" << endl;
}

void
testWithOutstandingRequest(TestIntfPrx p, bool oneway)
{
    cout << "testing the inactivity timeout with an outstanding " << (oneway ? "one-way" : "two-way") << " request... "
         << flush;
    if (oneway)
    {
        p = p->ice_oneway();
    }

    p->ice_ping();
    ConnectionPtr connection = p->ice_getCachedConnection();
    test(connection);

    // The inactivity timeout is 1s on the client side and 2s on the server side; 2.5 seconds tests both sides.
    p->sleep(2500);
    if (oneway)
    {
        this_thread::sleep_for(chrono::milliseconds(2500));
    }
    p->ice_ping();
    ConnectionPtr connection2 = p->ice_getCachedConnection();

    if (oneway)
    {
        // With a oneway invocation, the inactivity timeout on the client side shuts down the connection.
        test(connection2 != connection);
    }
    else
    {
        test(connection2 == connection);
    }
    cout << "ok" << endl;
}

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    string proxyString = "test: " + helper->getTestEndpoint();
    TestIntfPrx p(communicator, proxyString);

    string proxyString1s = "test: " + helper->getTestEndpoint(1);

    testClientInactivityTimeout(p);
    testServerInactivityTimeout(proxyString1s, communicator->getProperties());
    testWithOutstandingRequest(p, false);
    testWithOutstandingRequest(p, true);

    p->shutdown();
}
