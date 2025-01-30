// Copyright (c) ZeroC, Inc.

#include "Glacier2/Router.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

class Client final : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    Ice::CommunicatorHolder ich = initialize(argc, argv, properties);
    const auto& communicator = ich.communicator();

    Glacier2::RouterPrx router(communicator, "Glacier2/router:" + getTestEndpoint("tcp"));
    communicator->setDefaultRouter(router);

    //
    // First try to create a non ssl sessions.
    //
    cout << "creating non-ssl session with tcp connection... ";
    try
    {
        auto session = router->createSession("nossl", "");
        session->ice_ping();
        router->destroySession();
    }
    catch (const Glacier2::PermissionDeniedException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    cout << "creating ssl session with tcp connection... ";
    try
    {
        auto session = router->createSessionFromSecureConnection();
        test(false);
    }
    catch (const Glacier2::PermissionDeniedException&)
    {
    }
    cout << "ok" << endl;

    //
    // Switch to using the SSL router. First, clear the router. Then
    // set a new SSL based router.
    //
    communicator->setDefaultRouter(nullopt);
    router = Glacier2::RouterPrx(communicator, "Glacier2/router:" + getTestEndpoint(1, "ssl"));
    communicator->setDefaultRouter(router);

    //
    // Next try to create a non ssl session. This should succeed.
    //
    cout << "creating non-ssl session with ssl connection... ";
    try
    {
        auto session = router->createSession("ssl", "");
        session->ice_ping();
        router->destroySession();
    }
    catch (const Glacier2::PermissionDeniedException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    cout << "creating ssl session with ssl connection... ";
    try
    {
        auto session = router->createSessionFromSecureConnection();
        session->ice_ping();
        router->destroySession();
    }
    catch (const Glacier2::PermissionDeniedException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    communicator->setDefaultRouter(nullopt);
    auto process = checkedCast<Ice::ProcessPrx>(
        communicator->stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(2, "tcp")));
    process->shutdown();
}

DEFINE_TEST(Client)
