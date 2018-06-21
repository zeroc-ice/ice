// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

namespace
{

Ice::IPConnectionInfoPtr
getIPConnectionInfo(const Ice::ConnectionInfoPtr& info)
{
    for(Ice::ConnectionInfoPtr p = info; p; p = p->underlying)
    {
        Ice::IPConnectionInfoPtr ipInfo = ICE_DYNAMIC_CAST(Ice::IPConnectionInfo, p);
        if(ipInfo)
        {
            return ipInfo;
        }
    }
    return ICE_NULLPTR;
}

}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string sref = "test:" + helper->getTestEndpoint();
    Ice::ObjectPrxPtr obj = communicator->stringToProxy(sref);
    test(obj);

    int proxyPort = communicator->getProperties()->getPropertyAsInt("Ice.HTTPProxyPort");
    if(proxyPort == 0)
    {
        proxyPort = communicator->getProperties()->getPropertyAsInt("Ice.SOCKSProxyPort");
    }

    TestIntfPrxPtr test = ICE_CHECKED_CAST(TestIntfPrx, obj);
    test(test);

    cout << "testing connection... " << flush;
    {
        test->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing connection information... " << flush;
    {
        Ice::IPConnectionInfoPtr info = getIPConnectionInfo(test->ice_getConnection()->getInfo());
        test(info->remotePort == proxyPort); // make sure we are connected to the proxy port.
    }
    cout << "ok" << endl;

    cout << "shutting down server... " << flush;
    {
        test->shutdown();
    }
    cout << "ok" << endl;

    cout << "testing connection failure... " << flush;
    {
        try
        {
            test->ice_ping();
            test(false);
        }
        catch(const Ice::LocalException&)
        {
        }
    }
    cout << "ok" << endl;
}
