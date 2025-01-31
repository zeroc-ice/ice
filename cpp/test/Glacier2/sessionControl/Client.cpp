// Copyright (c) ZeroC, Inc.

#include "Glacier2/Router.h"
#include "Ice/Ice.h"
#include "Session.h"
#include "TestHelper.h"

#include <chrono>
#include <future>

using namespace std;
using namespace Ice;
using namespace Test;

class SessionControlClient final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
SessionControlClient::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);

    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    properties->setProperty("Ice.RetryIntervals", "-1");
    properties->setProperty("Ice.Warn.Connections", "0");

    Ice::CommunicatorHolder ich = initialize(argc, argv, properties);
    const auto& communicator = ich.communicator();

    cout << "setting router... " << flush;
    Glacier2::RouterPrx router(communicator, "Glacier2/router:" + getTestEndpoint(50));
    communicator->setDefaultRouter(router);
    cout << "ok" << endl;

    cout << "creating session... " << flush;
    auto session = uncheckedCast<Test::SessionPrx>(router->createSession("userid", "abc123"));
    test(session);
    cout << "ok" << endl;

    cout << "testing destroy... " << flush;
    ConnectionPtr connection = session->ice_getConnection();
    promise<void> connectionClosed;
    connection->setCloseCallback([&connectionClosed](const ConnectionPtr&) { connectionClosed.set_value(); });

    try
    {
        session->destroyFromClient();
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }

    // Make sure destroy closes the connection to the router.
    test(connectionClosed.get_future().wait_for(chrono::milliseconds(100)) == future_status::ready);
    cout << "ok" << endl;

    cout << "testing create exceptions... " << flush;
    try
    {
        router->createSession("rejectme", "abc123");
        test(false);
    }
    catch (const Glacier2::CannotCreateSessionException&)
    {
    }
    try
    {
        router->createSession("localexception", "abc123");
        test(false);
    }
    catch (const Glacier2::CannotCreateSessionException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing shutdown... " << flush;
    session = uncheckedCast<Test::SessionPrx>(router->createSession("userid", "abc123"));
    session->shutdown();
    communicator->setDefaultRouter(nullopt);
    Ice::ProcessPrx process(communicator, "Glacier2/admin -f Process:" + getTestEndpoint(51));
    process->shutdown();
    try
    {
        process->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
        cout << "ok" << endl;
    }
}

DEFINE_TEST(SessionControlClient)
