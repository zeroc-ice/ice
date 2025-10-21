// Copyright (c) ZeroC, Inc.

#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

// The client and server have the same idle timeout (1s) and both side enable the idle check (the default). We
// verify that the server's idle check does not abort a back-pressured connection (a connection the server doesn't
// read from). We use an OA with a MaxDispatches = 1 and a blocking dispatch to back-pressure the connection.
void
testIdleCheckDoesNotAbortBackPressuredConnection(const TestIntfPrx& p)
{
    cout << "testing that the idle check does not abort a back-pressured connection... " << flush;
    p->ice_ping();
    ConnectionPtr connection = p->ice_getCachedConnection();
    test(connection);
    p->sleep(2000); // the implementation in the server sleeps synchronously for 2,000ms
    cout << "ok" << endl;
}

// We verify that the idle check aborts the connection when the connection (here server connection) remains idle for
// longer than idle timeout. Here, the server has an idle timeout of 1s and idle checks enabled. We intentionally
// misconfigure the client with an idle timeout of 3s to send heartbeats every 1.5s, which is too long to prevent the
// server from aborting the connection.
void
testConnectionAbortedByIdleCheck(const string& proxyString, const PropertiesPtr& properties)
{
    cout << "testing that the idle check aborts a connection that does not receive anything for 1s... " << flush;

    // Create a new communicator with the desired properties.
    Ice::InitializationData initData;
    initData.properties = properties->clone();
    initData.properties->setProperty("Ice.Connection.Client.IdleTimeout", "3");
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    TestHelper::updateLogFileProperty(initData.properties, "-idleTimeout=3s");
    installTransport(initData);
    Ice::CommunicatorHolder holder = initialize(initData);
    TestIntfPrx p(holder.communicator(), proxyString);

    ConnectionPtr connection = p->ice_getConnection();
    test(connection);

    // The idle check on the server side aborts the connection because it doesn't get a heartbeat in a timely fashion.
    try
    {
        p->sleep(2000); // the implementation in the server sleeps for 2,000ms
        test(false);    // we expect the server to abort the connection after about 1 second.
    }
    catch (const ConnectionLostException&)
    {
        // Expected
    }
    cout << "ok" << endl;
}

// Verifies the behavior with the idle check enabled or disabled when the client and the server have mismatched idle
// timeouts (here: 3s on the server side and 1s on the client side).
void
testEnableDisableIdleCheck(bool enabled, const string& proxyString, const PropertiesPtr& properties)
{
    cout << "testing connection with idle check " << (enabled ? "enabled" : "disabled") << "... " << flush;

    // Create a new communicator with the desired properties.
    Ice::InitializationData initData;
    initData.properties = properties->clone();
    initData.properties->setProperty("Ice.Connection.Client.IdleTimeout", "1");
    initData.properties->setProperty("Ice.Connection.Client.EnableIdleCheck", enabled ? "1" : "0");
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    TestHelper::updateLogFileProperty(initData.properties, "-idleTimeout=1s");
    installTransport(initData);
    Ice::CommunicatorHolder holder = initialize(initData);
    TestIntfPrx p(holder.communicator(), proxyString);

    ConnectionPtr connection = p->ice_getConnection();
    test(connection);
    try
    {
        p->sleep(2000); // the implementation in the server sleeps for 2,000ms
        test(!enabled);
    }
    catch (const ConnectionAbortedException& ex)
    {
        test(!ex.closedByApplication());
        test(enabled);
    }

    cout << "ok" << endl;
}

void
testNoIdleTimeout(const string& proxyString, const PropertiesPtr& properties)
{
    cout << "testing connection with idle timeout set to 0... " << flush;

    // Create a new communicator with the desired properties.
    Ice::InitializationData initData;
    initData.properties = properties->clone();
    initData.properties->setProperty("Ice.Connection.Client.IdleTimeout", "0");
    TestHelper::updateLogFileProperty(initData.properties, "-idleTimeout=0");
    installTransport(initData);
    Ice::CommunicatorHolder holder = initialize(initData);
    TestIntfPrx p(holder.communicator(), proxyString);

    ConnectionPtr connection = p->ice_getConnection();
    test(connection);
    p->sleep(2000); // the implementation in the server sleeps for 2,000ms
    connection->close().get();
    cout << "ok" << endl;
}

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    string proxyString = "test: " + helper->getTestEndpoint();
    TestIntfPrx p(communicator, proxyString);

    string proxyStringDefaultMax = "test: " + helper->getTestEndpoint(1);
    string proxyString3s = "test: " + helper->getTestEndpoint(2);
    string proxyStringNoIdleTimeout = "test: " + helper->getTestEndpoint(3);

    testIdleCheckDoesNotAbortBackPressuredConnection(p);
    testConnectionAbortedByIdleCheck(proxyStringDefaultMax, communicator->getProperties());
    testEnableDisableIdleCheck(true, proxyString3s, communicator->getProperties());
    testEnableDisableIdleCheck(false, proxyString3s, communicator->getProperties());
    testNoIdleTimeout(proxyStringNoIdleTimeout, communicator->getProperties());

    p->shutdown();
}
