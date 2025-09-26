// Copyright (c) ZeroC, Inc.

#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

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

    // The inactivity timeout is 3s on the client side and 5s on the server side. 4 seconds tests the client side.
    this_thread::sleep_for(chrono::seconds(4));
    p->ice_ping();
    ConnectionPtr connection2 = p->ice_getCachedConnection();
    test(connection2 != connection);
    connection2->close().get();
    cout << "ok" << endl;
}

void
testServerInactivityTimeout(const string& proxyString, const PropertiesPtr& properties, bool disableCheck)
{
    if (disableCheck)
    {
        cout << "testing connection with a disabled server-side inactivity check... " << flush;
    }
    else
    {
        cout << "testing that the server side inactivity timeout shuts down the connection... " << flush;
    }

    // Create a new communicator with the desired properties.
    Ice::InitializationData initData;
    initData.properties = properties->clone();
    initData.properties->setProperty("Ice.Connection.Client.InactivityTimeout", "5");
    installTransport(initData);
    Ice::CommunicatorHolder holder = initialize(initData);
    TestIntfPrx p(holder.communicator(), proxyString);

    p->ice_ping();
    if (disableCheck)
    {
        p->disableInactivityCheck();
    }

    ConnectionPtr connection = p->ice_getCachedConnection();
    test(connection);

    // The inactivity timeout is 5s on the client side and 3s on the server side. 4 seconds tests the server side.
    this_thread::sleep_for(chrono::seconds(4));
    p->ice_ping();
    ConnectionPtr connection2 = p->ice_getCachedConnection();

    if (disableCheck)
    {
        // With a disabled inactivity check, the connections should be the same.
        test(connection2 == connection);
    }
    else
    {
        // When the inactivity check is enabled, the connections should be different because the server closed the
        // first connection.
        test(connection2 != connection);
    }
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

    // The inactivity timeout is 3s on the client side and 5s on the server side; 4 seconds tests only the client-side.
    p->sleep(4000); // two-way blocks for 4 seconds; one-way is non-blocking
    if (oneway)
    {
        this_thread::sleep_for(chrono::milliseconds(4000));
    }
    p->ice_ping();
    ConnectionPtr connection2 = p->ice_getCachedConnection();

    if (oneway)
    {
        // With a oneway invocation, the inactivity timeout on the client side shut down the first connection.
        test(connection2 != connection);
    }
    else
    {
        // With a two-way invocation, the inactivity timeout should not shutdown any connection.
        test(connection2 == connection);
    }
    connection2->close().get();
    cout << "ok" << endl;
}

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    string proxyString = "test: " + helper->getTestEndpoint();
    TestIntfPrx p(communicator, proxyString);

    string proxyString3s = "test: " + helper->getTestEndpoint(1);

    testClientInactivityTimeout(p);
    testServerInactivityTimeout(proxyString3s, communicator->getProperties(), false);
    testServerInactivityTimeout(proxyString3s, communicator->getProperties(), true);
    testWithOutstandingRequest(p, false);
    testWithOutstandingRequest(p, true);

    p->shutdown();
}
