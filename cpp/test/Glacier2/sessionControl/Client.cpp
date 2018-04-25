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
#include <Session.h>
#include <TestCommon.h>
#include <set>

using namespace std;
using namespace Ice;
using namespace Test;

class SessionControlClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Ice::InitializationData initData = getTestInitData(argc, argv);

    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    initData.properties->setProperty("Ice.RetryIntervals", "-1");
    initData.properties->setProperty("Ice.Warn.Connections", "0");

    SessionControlClient app;
    return app.main(argc, argv, initData);
}

int
SessionControlClient::run(int, char**)
{
    cout << "getting router... " << flush;
    ObjectPrx routerBase = communicator()->stringToProxy("Glacier2/router:" + getTestEndpoint(communicator(), 50));
    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(routerBase);
    test(router);
    communicator()->setDefaultRouter(router);
    cout << "ok" << endl;

    cout << "creating session... " << flush;
    Glacier2::SessionPrx sessionBase = router->createSession("userid", "abc123");
    Test::SessionPrx session = Test::SessionPrx::uncheckedCast(sessionBase);
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
    session = Test::SessionPrx::uncheckedCast(router->createSession("userid", "abc123"));
    session->shutdown();
    communicator()->setDefaultRouter(0);
    ObjectPrx processBase = communicator()->stringToProxy("Glacier2/admin -f Process:" +
                                                          getTestEndpoint(communicator(), 51));
    Ice::ProcessPrx process = Ice::ProcessPrx::checkedCast(processBase);
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

    return EXIT_SUCCESS;
}
