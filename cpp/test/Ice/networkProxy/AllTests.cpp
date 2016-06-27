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
allTests(const Ice::CommunicatorPtr& communicator)
{
    string sref = "test:default -p 12010";
    Ice::ObjectPrxPtr obj = communicator->stringToProxy(sref);
    test(obj);

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
        test(info->remotePort == 12030 || info->remotePort == 12031); // make sure we are connected to the proxy port.
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
