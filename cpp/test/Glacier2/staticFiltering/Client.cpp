// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Backend.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

class AttackClient : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
AttackClient::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    properties->setProperty("Ice.RetryIntervals", "-1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    ObjectPrx routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(50));
    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(routerBase);
    test(router);
    communicator->setDefaultRouter(router);

    PropertyDict::const_iterator p;

    PropertyDict badProxies = communicator->getProperties()->getPropertiesForPrefix("Reject.Proxy.");
    for(p = badProxies.begin(); p != badProxies.end(); ++p)
    {
        try
        {
            Glacier2::SessionPrx session = router->createSession("userid", "abc123");
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
            test(false);
        }
        BackendPrx backend = BackendPrx::uncheckedCast(communicator->stringToProxy(p->second));
        try
        {
            backend->ice_ping();
            cerr << "Test failed on : " << p->second << endl;
            test(false);
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
            test(false);
        }
    }

    PropertyDict goodProxies = communicator->getProperties()->getPropertiesForPrefix("Accept.Proxy.");
    for(p = goodProxies.begin(); p != goodProxies.end(); ++p)
    {
        try
        {
            Glacier2::SessionPrx session = router->createSession("userid", "abc123");
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
            test(false);
        }
        BackendPrx backend = BackendPrx::uncheckedCast(communicator->stringToProxy(p->second));
        try
        {
            backend->ice_ping();
        }
        catch(const LocalException& ex)
        {
            cerr << p->second << endl;
            cerr << ex << endl;
            test(false);
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
    communicator->setDefaultRouter(0);
    try
    {
        BackendPrx backend = BackendPrx::checkedCast(communicator->stringToProxy("dummy:tcp -p 12010"));
        backend->shutdown();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }

    ObjectPrx processBase = communicator->stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
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
}

DEFINE_TEST(AttackClient)
