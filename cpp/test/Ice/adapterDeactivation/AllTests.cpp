// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

TestIntfPrx
allTests(const CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    ObjectPrx base = communicator->stringToProxy("test:default -p 12010");
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;
#ifdef ICE_OS_WINRT
    bool winrt = true;
#else
    bool winrt = false;
#endif

    {
        if(!winrt || (communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
                      communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss"))
        {
            string host = communicator->getProperties()->getPropertyAsIntWithDefault("Ice.IPv6", 0) == 0 ?
                "127.0.0.1" : "\"0:0:0:0:0:0:0:1\"";
            cout << "creating/destroying/recreating object adapter... " << flush;
            ObjectAdapterPtr adapter =
                communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default -h " + host);
            try
            {
                communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default -h " + host);
                test(false);
            }
            catch(const AlreadyRegisteredException&)
            {
            }
            adapter->destroy();

            //
            // Use a different port than the first adapter to avoid an "address already in use" error.
            //
            adapter = communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default -h " + host);
            adapter->destroy();
            cout << "ok" << endl;
        }
    }

    cout << "creating/activating/deactivating object adapter in one operation... " << flush;
    obj->transient();
    obj->end_transient(obj->begin_transient());
    cout << "ok" << endl;

    {
        cout << "testing connection closure... " << flush;
        for(int i = 0; i < 10; ++i)
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            Ice::CommunicatorPtr comm = Ice::initialize(initData);
            comm->stringToProxy("test:default -p 12010")->begin_ice_ping();
            comm->destroy();
        }
        cout << "ok" << endl;
    }

    cout << "deactivating object adapter in the server... " << flush;
    obj->deactivate();
    cout << "ok" << endl;

    cout << "testing whether server is gone... " << flush;
    try
    {
        obj->ice_ping();
        test(false);
    }
    catch(const LocalException&)
    {
        cout << "ok" << endl;
    }

    return obj;
}
