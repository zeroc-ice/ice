//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Backend.h>
#include <TestHelper.h>
#include <set>
#include <random>

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
    cout << "getting router... " << flush;
    auto routerBase = communicator->stringToProxy("Glacier2/router:" + getTestEndpoint(50));
    auto router = checkedCast<Glacier2::RouterPrx>(routerBase);
    test(router);
    communicator->setDefaultRouter(router);
    cout << "ok" << endl;

    cout << "creating session... " << flush;
    auto session = router->createSession("userid", "abc123");
    cout << "ok" << endl;

    cout << "making thousands of invocations on proxies... " << flush;
    auto backendBase = communicator->stringToProxy("dummy:" + getTestEndpoint());
    auto backend = checkedCast<BackendPrx>(backendBase);
    backend->ice_ping();

    set<shared_ptr<BackendPrx>, TargetCompare<shared_ptr<BackendPrx>, std::less>> backends;

    string msg;
    for(int i = 1; i <= 10000; ++i)
    {
        if(i % 100 == 0)
        {
            if(!msg.empty())
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
        for(p = ident.name.begin(); p != ident.name.end(); ++p)
        {
            *p = static_cast<char>('A' + rd() % 26);
        }

        ident.category.resize(rd() % 2);
        for(p = ident.category.begin(); p != ident.category.end(); ++p)
        {
            *p = static_cast<char>('a' + rd() % 26);
        }

        auto newBackend = uncheckedCast<BackendPrx>(backendBase->ice_identity(ident));

        auto q = backends.find(newBackend);

        if(q == backends.end())
        {
            backends.insert(newBackend);
            backend = newBackend;
        }
        else
        {
            backend = *q;
        }

        backend->ice_ping();
    }
    cout << string(msg.size(), '\b') << string(msg.size(), ' ') << string(msg.size(), '\b');
    cout << "ok" << endl;

    cout << "testing server and router shutdown... " << flush;
    backend->shutdown();
    communicator->setDefaultRouter(0);
    auto adminBase = communicator->stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
    auto process = checkedCast<Ice::ProcessPrx>(adminBase);
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
