// Copyright (c) ZeroC, Inc.

#include "Backend.h"
#include "Glacier2/Router.h"
#include "Ice/Ice.h"
#include "TestHelper.h"
#include <random>
#include <set>

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

    Ice::CommunicatorHolder ich = initialize(argc, argv, properties);
    const auto& communicator = ich.communicator();
    cout << "setting router... " << flush;
    Glacier2::RouterPrx router(communicator, "Glacier2/router:" + getTestEndpoint(50));
    communicator->setDefaultRouter(router);
    cout << "ok" << endl;

    cout << "creating session... " << flush;
    auto session = router->createSession("userid", "abc123");
    cout << "ok" << endl;

    cout << "making thousands of invocations on proxies... " << flush;
    BackendPrx backend(communicator, "dummy:" + getTestEndpoint());
    backend->ice_ping();

    set<optional<BackendPrx>> backends;

    string msg;
    for (int i = 1; i <= 10000; ++i)
    {
        if (i % 100 == 0)
        {
            if (!msg.empty())
            {
                cout << string(msg.size(), '\b');
            }
            ostringstream s;
            s << i;
            msg = s.str();
            cout << msg << flush;
        }

        Identity ident;
        string::iterator p;
        random_device rd;

        ident.name.resize(1);
        for (p = ident.name.begin(); p != ident.name.end(); ++p)
        {
            *p = static_cast<char>('A' + rd() % 26);
        }

        ident.category.resize(rd() % 2);
        for (p = ident.category.begin(); p != ident.category.end(); ++p)
        {
            *p = static_cast<char>('a' + rd() % 26);
        }

        auto newBackend = backend->ice_identity<BackendPrx>(ident);

        auto q = backends.find(newBackend);

        if (q == backends.end())
        {
            backends.insert(newBackend);
            backend = newBackend;
        }
        else
        {
            backend = q->value();
        }

        backend->ice_ping();
    }
    cout << string(msg.size(), '\b') << string(msg.size(), ' ') << string(msg.size(), '\b');
    cout << "ok" << endl;

    cout << "testing server and router shutdown... " << flush;
    backend->shutdown();
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

DEFINE_TEST(AttackClient)
