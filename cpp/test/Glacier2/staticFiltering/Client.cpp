// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/Router.h>
#include <Backend.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace Test;

class AttackClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    Ice::InitializationData initData = getTestInitData(argc, argv);

    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    initData.properties->setProperty("Ice.RetryIntervals", "-1");

    AttackClient app;
    return app.main(argc, argv, initData);
}

int
AttackClient::run(int, char**)
{
    ObjectPrx routerBase = communicator()->stringToProxy("Glacier2/router:" + getTestEndpoint(communicator(), 10));
    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(routerBase);
    test(router);
    communicator()->setDefaultRouter(router);

    PropertyDict::const_iterator p;

    PropertyDict badProxies = communicator()->getProperties()->getPropertiesForPrefix("Reject.Proxy.");
    for(p = badProxies.begin(); p != badProxies.end(); ++p)
    {
        try
        {
            Glacier2::SessionPrx session = router->createSession("userid", "abc123");
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
            test("Unable to create new session" == 0);
        }
        BackendPrx backend = BackendPrx::uncheckedCast(communicator()->stringToProxy(p->second));
        try
        {
            backend->ice_ping();
            cerr << "Test failed on : " << p->second << endl;
            test("Expected exception" == 0);
        }
        catch(const ConnectionLostException&)
        {
            //
            // This is ok.
            //
        }
        catch(const CloseConnectionException&)
        {
            //
            // This is also ok.
            //
        }
        catch(const ObjectNotExistException&)
        {
            //
            // This is ok for non-address filters.
            //
            try
            {
                router->destroySession();
            }
            catch(...)
            {
            }
        }
        catch(const LocalException& e)
        {
            cerr << e << endl;
            test("Unexpected local exception" == 0);
        }
    }

    PropertyDict goodProxies = communicator()->getProperties()->getPropertiesForPrefix("Accept.Proxy.");
    for(p = goodProxies.begin(); p != goodProxies.end(); ++p)
    {
        try
        {
            Glacier2::SessionPrx session = router->createSession("userid", "abc123");
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
            test("Unable to create new session" == 0);
        }
        BackendPrx backend = BackendPrx::uncheckedCast(communicator()->stringToProxy(p->second));
        try
        {
            backend->ice_ping();
        }
        catch(const LocalException& ex)
        {
            cerr << p->second << endl;
            cerr << ex << endl;
            test("Unexpected local exception" == 0);
        }
        try
        {
            router->destroySession();
        }
        catch(const LocalException&)
        {
            //
            // Expected.
            //
        }
    }

    //
    // Stop using router and communicate with backend and router directly
    // to shut things down.
    //
    communicator()->setDefaultRouter(0);
    try
    {
        BackendPrx backend = BackendPrx::checkedCast(communicator()->stringToProxy("dummy:tcp -p 12010"));
        backend->shutdown();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }

    ObjectPrx processBase = communicator()->stringToProxy("Glacier2/admin -f Process:" +
                                                          getTestEndpoint(communicator(), 11, "tcp"));
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
