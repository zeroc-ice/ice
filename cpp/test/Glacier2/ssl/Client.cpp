// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/Router.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
//using namespace Test;

class CallbackClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    //
    // We must disable connection warnings, because we attempt to ping
    // the router before session establishment, as well as after
    // session destruction. Both will cause a ConnectionLostException.
    //
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties(argc, argv);
    initData.properties->setProperty("Ice.Warn.Connections", "0");

    CallbackClient app;
    return app.main(argc, argv, initData);
}

int
CallbackClient::run(int argc, char* argv[])
{
    Glacier2::RouterPrx router = Glacier2::RouterPrx::uncheckedCast(
        communicator()->stringToProxy("Glacier2/router:tcp -h 127.0.0.1 -p 12347 -t 10000"));
    communicator()->setDefaultRouter(router);

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
    catch(const Ice::ConnectionLostException&)
    {
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
    communicator()->setDefaultRouter(Glacier2::RouterPrx());
    router = Glacier2::RouterPrx::uncheckedCast(
        communicator()->stringToProxy("Glacier2/router:ssl -h 127.0.0.1 -p 12348 -t 10000"));
    communicator()->setDefaultRouter(router);

    //
    // Next try to create a non ssl session. This should succeed.
    //
    cout << "creating non-ssl session with ssl connection... ";
    try
    {
        Glacier2::SessionPrx session = router->createSession("nossl", "");
        session->ice_ping();
        router->destroySession();
    }
    catch(const Ice::ConnectionLostException&)
    {
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
    catch(const Ice::ConnectionLostException&)
    {
    }
    catch(const Glacier2::PermissionDeniedException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    communicator()->setDefaultRouter(0);
    Glacier2::AdminPrx admin = Glacier2::AdminPrx::checkedCast(
        communicator()->stringToProxy("Glacier2/admin:tcp -h 127.0.0.1 -p 12349 -t 10000"));
    admin->shutdown();
    
    return EXIT_SUCCESS;
}
