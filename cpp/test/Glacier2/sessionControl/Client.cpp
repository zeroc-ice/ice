//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Session.h>
#include <TestHelper.h>
#include <set>

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

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    cout << "getting router... " << flush;
    auto routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(50));
    auto router = checkedCast<Glacier2::RouterPrx>(routerBase);
    test(router);
    communicator->setDefaultRouter(router);
    cout << "ok" << endl;

    cout << "creating session... " << flush;
    auto sessionBase = router->createSession("userid", "abc123");
    auto session = uncheckedCast<Test::SessionPrx>(sessionBase);
    test(session);
    cout << "ok" << endl;

    cout << "testing destroy... " << flush;
    try
    {
        session->destroyFromClient();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    try
    {
        session->ice_ping();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing create exceptions... " << flush;
    try
    {
        router->createSession("rejectme", "abc123");
        test(false);
    }
    catch(const Glacier2::CannotCreateSessionException&)
    {
    }
    try
    {
        router->createSession("localexception", "abc123");
        test(false);
    }
    catch(const Glacier2::CannotCreateSessionException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing shutdown... " << flush;
    session = uncheckedCast<Test::SessionPrx>(router->createSession("userid", "abc123"));
    session->shutdown();
    communicator->setDefaultRouter(0);
    auto processBase = communicator->stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
    auto process = checkedCast<Ice::ProcessPrx>(processBase);
    test(process);
    process->shutdown();
    try
    {
        process->ice_ping();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
        cout << "ok" << endl;
    }
}

DEFINE_TEST(SessionControlClient)
