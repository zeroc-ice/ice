// Copyright (c) ZeroC, Inc.

#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>
#include <vector>

using namespace std;
using namespace Ice;
using namespace Test;

// Verify that we can create connectionCount connections and send a ping on each connection.
void
testCreateConnections(TestIntfPrx p, int connectionCount, const function<void()>& postCloseDelay)
{
    cout << "testing the creation of " << connectionCount << " connections... " << flush;
    vector<ConnectionPtr> connectionList;
    for (int i = 0; i < connectionCount; ++i)
    {
        p = p->ice_connectionId("connection-" + to_string(i));
        p->ice_ping();
        connectionList.push_back(p->ice_getCachedConnection());
    }

    // Close all connections
    for (const auto& connection : connectionList)
    {
        connection->close().get();
    }

    if (postCloseDelay)
    {
        postCloseDelay();
    }

    cout << "ok" << endl;
}

// Verify that we can create connectionCount connections and send a ping on each connection.
void
testCreateConnectionsWithMax(TestIntfPrx p, int max, const function<void()>& postCloseDelay)
{
    cout << "testing the creation of " << max << " connections with connection lost at " << (max + 1) << "... "
         << flush;
    vector<ConnectionPtr> connectionList;
    for (int i = 0; i < max; ++i)
    {
        p = p->ice_connectionId("connection-" + to_string(i));
        p->ice_ping();
        connectionList.push_back(p->ice_getCachedConnection());
    }

    p = p->ice_connectionId("connection-" + to_string(max));

    try
    {
        p->ice_ping();
        test(false);
    }
    catch (const Ice::ConnectionLostException&)
    {
        // Expected
    }

    // Close all connections
    for (const auto& connection : connectionList)
    {
        connection->close().get();
    }

    if (postCloseDelay)
    {
        postCloseDelay();
    }

    cout << "ok" << endl;
}

// Verify that we can create max connections, then connection lost, then recover.
void
testCreateConnectionsWithMaxAndRecovery(TestIntfPrx p, int max, const function<void()>& postCloseDelay)
{
    cout << "testing the creation of " << max << " connections with connection lost at " << (max + 1)
         << " then recovery... " << flush;
    vector<ConnectionPtr> connectionList;
    for (int i = 0; i < max; ++i)
    {
        p = p->ice_connectionId("connection-" + to_string(i));
        p->ice_ping();
        connectionList.push_back(p->ice_getCachedConnection());
    }

    p = p->ice_connectionId("connection-" + to_string(max));

    try
    {
        p->ice_ping();
        test(false);
    }
    catch (const Ice::ConnectionLostException&)
    {
        // Expected
    }

    // Close one connection
    connectionList.front()->close().get();
    connectionList.erase(connectionList.begin());

    if (postCloseDelay)
    {
        postCloseDelay();
    }
    else
    {
        // We need to wait a tiny bit to let the server remove the connection from its incoming connection factory.
        this_thread::sleep_for(10ms);
    }

    // Try again
    try
    {
        p->ice_ping();
    }
    catch (const Ice::ConnectionLostException& ex)
    {
        cout << "Unexpected exception: " << ex << endl;
        test(false);
    }

    connectionList.push_back(p->ice_getCachedConnection());

    // Close all connections
    for (const auto& connection : connectionList)
    {
        connection->close().get();
    }

    if (postCloseDelay)
    {
        postCloseDelay();
    }
    cout << "ok" << endl;
}

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    string proxyString = "test: " + helper->getTestEndpoint();
    TestIntfPrx p(communicator, proxyString);

    string proxyStringMax10 = "test: " + helper->getTestEndpoint(1);
    TestIntfPrx pMax10(communicator, proxyStringMax10);

    string transport = helper->getTestProtocol();

    // When the transport is WS or WSS, we need to wait a little bit: the server closes the connection after it
    // gets a transport frame from the client.
    function<void()> postCloseDelay;
    if (transport.find("ws") == 0)
    {
        postCloseDelay = []() { this_thread::sleep_for(50ms); };
    }

    testCreateConnections(p, 100, postCloseDelay);
    testCreateConnectionsWithMax(pMax10, 10, postCloseDelay);
    testCreateConnectionsWithMaxAndRecovery(pMax10, 10, postCloseDelay);

    p->shutdown();
}
