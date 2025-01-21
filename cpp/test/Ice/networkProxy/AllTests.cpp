// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

namespace
{
    Ice::IPConnectionInfoPtr getIPConnectionInfo(const Ice::ConnectionInfoPtr& info)
    {
        for (Ice::ConnectionInfoPtr p = info; p; p = p->underlying)
        {
            Ice::IPConnectionInfoPtr ipInfo = dynamic_pointer_cast<Ice::IPConnectionInfo>(p);
            if (ipInfo)
            {
                return ipInfo;
            }
        }
        return nullptr;
    }
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();

    int proxyPort = communicator->getProperties()->getIcePropertyAsInt("Ice.HTTPProxyPort");
    if (proxyPort == 1080) // default, i.e. most likely not set
    {
        proxyPort = communicator->getProperties()->getIcePropertyAsInt("Ice.SOCKSProxyPort");
    }

    TestIntfPrx testPrx(communicator, "test:" + helper->getTestEndpoint());

    cout << "testing connection... " << flush;
    {
        testPrx->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing connection information... " << flush;
    {
        Ice::IPConnectionInfoPtr info = getIPConnectionInfo(testPrx->ice_getConnection()->getInfo());
        test(info->remotePort == proxyPort); // make sure we are connected to the proxy port.
    }
    cout << "ok" << endl;

    cout << "shutting down server... " << flush;
    {
        testPrx->shutdown();
    }
    cout << "ok" << endl;

    cout << "testing connection failure... " << flush;
    {
        try
        {
            testPrx->ice_ping();
            test(false);
        }
        catch (const Ice::LocalException&)
        {
        }
    }
    cout << "ok" << endl;
}
