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
testWithOutstandingOnewaySend(TestIntfPrx p, TestIntfControllerPrx controller)
{
    cout << "testing the inactivity timeout with outstanding one-way request to send... " << flush;

    p = p->ice_oneway();
    p->ice_ping();
    ConnectionPtr connection1 = p->ice_getCachedConnection();

    // Hold the server adapter and send 10 MB with a oneway request. The sending should block. The connection inactivity
    // timeout shouldn't be triggered while the connection is sending the payload.
    controller->holdAdapter();

    // Send the payload. The payload shouldn't be fully sent until the adapter is resumed.
    Ice::ByteSeq seq;
    seq.resize(10 * 1024 * 1024);
    promise<void> sent;
    auto future = sent.get_future();
    p->sendPayloadAsync(seq, []() {}, [](exception_ptr) { test(false); }, [&sent](bool) { sent.set_value(); });
    test(future.wait_for(chrono::milliseconds(100)) != future_status::ready);

    // The inactivity timeout is 1s on the client side.
    this_thread::sleep_for(chrono::seconds(2));

    // Resume the adapter and ensure the request is sent.
    test(future.wait_for(chrono::seconds(0)) != future_status::ready);
    controller->resumeAdapter();
    future.get();

    p->ice_ping();
    test(connection1 == p->ice_getCachedConnection());

    cout << "ok" << endl;
}

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();

    string proxyString = "test: " + helper->getTestEndpoint();
    TestIntfPrx p(communicator, proxyString);

    string proxyString1s = "test: " + helper->getTestEndpoint(1);

    string controllerProxyString = "testController: " + helper->getTestEndpoint(2);
    TestIntfControllerPrx controller(communicator, controllerProxyString);

    testClientInactivityTimeout(p);
    testServerInactivityTimeout(proxyString1s, communicator->getProperties());
    testWithOutstandingRequest(p, false);
    testWithOutstandingRequest(p, true);
    testWithOutstandingOnewaySend(p, controller);

    p->shutdown();
}
