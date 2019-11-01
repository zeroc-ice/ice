//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Backend.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

class AttackClient final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
AttackClient::run(int argc, char** argv)
{
    auto properties = createTestProperties(argc, argv);
    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    properties->setProperty("Ice.RetryIntervals", "-1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    auto routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(50));
    auto router = checkedCast<Glacier2::RouterPrx>(routerBase);
    test(router);
    communicator->setDefaultRouter(router);

    auto badProxies = communicator->getProperties()->getPropertiesForPrefix("Reject.Proxy.");
    for(const auto& p : badProxies)
    {
        try
        {
            auto session = router->createSession("userid", "abc123");
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
            test(false);
        }
        auto backend = uncheckedCast<BackendPrx>(communicator->stringToProxy(p.second));
        try
        {
            backend->ice_ping();
            cerr << "Test failed on : " << p.second << endl;
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

    auto goodProxies = communicator->getProperties()->getPropertiesForPrefix("Accept.Proxy.");
    for(const auto& p : goodProxies)
    {
        try
        {
            auto session = router->createSession("userid", "abc123");
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
            test(false);
        }
        auto backend = uncheckedCast<BackendPrx>(communicator->stringToProxy(p.second));
        try
        {
            backend->ice_ping();
        }
        catch(const LocalException& ex)
        {
            cerr << p.second << endl;
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
    communicator->setDefaultRouter(nullptr);
    try
    {
        auto backend = checkedCast<BackendPrx>(communicator->stringToProxy("dummy:tcp -p 12010"));
        backend->shutdown();
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }

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

DEFINE_TEST(AttackClient)
