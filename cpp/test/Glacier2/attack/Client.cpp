// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>
#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Backend.h>
#include <TestCommon.h>
#include <set>

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
    cout << "getting router... " << flush;
    ObjectPrx routerBase = communicator()->stringToProxy("Glacier2/router:" + getTestEndpoint(communicator(), 50));
    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(routerBase);
    test(router);
    communicator()->setDefaultRouter(router);
    cout << "ok" << endl;

    cout << "creating session... " << flush;
    Glacier2::SessionPrx session = router->createSession("userid", "abc123");
    cout << "ok" << endl;

    cout << "making thousands of invocations on proxies... " << flush;
    ObjectPrx backendBase = communicator()->stringToProxy("dummy:" + getTestEndpoint(communicator(), 0));
    BackendPrx backend = BackendPrx::uncheckedCast(backendBase);
    backend->ice_ping();

    set<BackendPrx> backends;

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

        ident.name.resize(1); // 1 + IceUtilInternal::random() % 2);
        for(p = ident.name.begin(); p != ident.name.end(); ++p)
        {
            *p = static_cast<char>('A' + IceUtilInternal::random() % 26);
        }

        ident.category.resize(IceUtilInternal::random() % 2);
        for(p = ident.category.begin(); p != ident.category.end(); ++p)
        {
            *p = static_cast<char>('a' + IceUtilInternal::random() % 26);
        }

        BackendPrx newBackend = BackendPrx::uncheckedCast(backendBase->ice_identity(ident));

        set<BackendPrx>::const_iterator q = backends.find(newBackend);

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
    communicator()->setDefaultRouter(0);
    ObjectPrx adminBase = communicator()->stringToProxy("Glacier2/admin -f Process:" +
                                                        getTestEndpoint(communicator(), 51));
    Ice::ProcessPrx process = Ice::ProcessPrx::checkedCast(adminBase);
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
