// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    Glacier2::RouterPrx router = Glacier2::RouterPrx::uncheckedCast(
        communicator->stringToProxy("Glacier2/router:" + getTestEndpoint("tcp")));
    communicator->setDefaultRouter(router);

    //
    // First try to create a non ssl sessions.
    //
    cout << "creating non-ssl session with tcp connection... ";
    try
    {
        Glacier2::SessionPrx session = router->createSession("nossl", "");
        session->ice_ping();
        router->destroySession();
    }
    catch(const Glacier2::PermissionDeniedException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    cout << "creating ssl session with tcp connection... ";
    try
    {
        Glacier2::SessionPrx session = router->createSessionFromSecureConnection();
        test(false);
    }
    catch(const Glacier2::PermissionDeniedException&)
    {
    }
    cout << "ok" << endl;

    //
    // Switch to using the SSL router. First, clear the router. Then
    // set a new SSL based router.
    //
    communicator->setDefaultRouter(Glacier2::RouterPrx());
    router = Glacier2::RouterPrx::uncheckedCast(
        communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(1, "ssl")));
    communicator->setDefaultRouter(router);

    //
    // Next try to create a non ssl session. This should succeed.
    //
    cout << "creating non-ssl session with ssl connection... ";
    try
    {
        Glacier2::SessionPrx session = router->createSession("ssl", "");
        session->ice_ping();
        router->destroySession();
    }
    catch(const Glacier2::PermissionDeniedException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    cout << "creating ssl session with ssl connection... ";
    try
    {
        Glacier2::SessionPrx session = router->createSessionFromSecureConnection();
        session->ice_ping();
        router->destroySession();
    }
    catch(const Glacier2::PermissionDeniedException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    communicator->setDefaultRouter(0);
    Ice::ProcessPrx process = Ice::ProcessPrx::checkedCast(
        communicator->stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(2, "tcp")));
    process->shutdown();
}

DEFINE_TEST(Client)
