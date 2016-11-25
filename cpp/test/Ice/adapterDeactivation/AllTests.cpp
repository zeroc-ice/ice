// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

TestIntfPrxPtr
allTests(const CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    ObjectPrxPtr base = communicator->stringToProxy("test:" + getTestEndpoint(communicator, 0));
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrxPtr obj = ICE_CHECKED_CAST(TestIntfPrx, base);
    test(obj);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(obj, base));
#else
    test(obj == base);
#endif
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
            cout << "creating/destroying/recreating object adapter... " << flush;
            ObjectAdapterPtr adpt = communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            try
            {
                communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                test(false);
            }
            catch(const AlreadyRegisteredException&)
            {
            }
            adpt->destroy();

            adpt = communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            adpt->destroy();
            cout << "ok" << endl;
        }
    }

    cout << "creating/activating/deactivating object adapter in one operation... " << flush;
    obj->transient();
#ifdef ICE_CPP11_MAPPING
    obj->transientAsync().get();
#else
    obj->end_transient(obj->begin_transient());
#endif
    cout << "ok" << endl;

    {
        cout << "testing connection closure... " << flush;
        for(int i = 0; i < 10; ++i)
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            Ice::CommunicatorHolder comm = Ice::initialize(initData);
#ifdef ICE_CPP11_MAPPING
            comm->stringToProxy("test:" + getTestEndpoint(communicator, 0))->ice_pingAsync();
#else
            comm->stringToProxy("test:" + getTestEndpoint(communicator, 0))->begin_ice_ping();
#endif
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
