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

void
allTests(const CommunicatorPtr& communicator, int num)
{
    vector<ControllerPrx> proxies;
    vector<ControllerPrx> indirectProxies;
    for(int i = 0; i < num; ++i)
    {
        {
            ostringstream os;
            os << "controller" << i;
            proxies.push_back(ControllerPrx::uncheckedCast(communicator->stringToProxy(os.str())));
        }
        {
            ostringstream os;
            os << "controller" << i << "@control" << i;
            indirectProxies.push_back(ControllerPrx::uncheckedCast(communicator->stringToProxy(os.str())));
        }
    }

    cout << "testing indirect proxies... " << flush;
    {
        for(vector<ControllerPrx>::const_iterator p = indirectProxies.begin(); p != indirectProxies.end(); ++p)
        {
            (*p)->ice_ping();
        }
    }
    cout << "ok" << endl;

    cout << "testing well-known proxies... " << flush;
    {
        for(vector<ControllerPrx>::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
        {
            (*p)->ice_ping();
        }
    }
    cout << "ok" << endl;

    cout << "testing object adapter registration... " << flush;
    {
        try
        {
            communicator->stringToProxy("object @ oa1")->ice_ping();
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        proxies[0]->activateObjectAdapter("oa", "oa1", "");

        try
        {
            communicator->stringToProxy("object @ oa1")->ice_ping();
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        proxies[0]->deactivateObjectAdapter("oa");

        try
        {
            communicator->stringToProxy("object @ oa1")->ice_ping();
        }
        catch(const Ice::NoEndpointException&)
        {
        }
    }
    cout << "ok" << endl;
    
    cout << "testing object adapter migration..." << flush;
    {
        proxies[0]->activateObjectAdapter("oa", "oa1", "");
        proxies[0]->addObject("oa", "object");
        communicator->stringToProxy("object @ oa1")->ice_ping();
        proxies[0]->removeObject("oa", "object");
        proxies[0]->deactivateObjectAdapter("oa");

        proxies[1]->activateObjectAdapter("oa", "oa1", "");
        proxies[1]->addObject("oa", "object");
        communicator->stringToProxy("object @ oa1")->ice_ping();
        proxies[1]->removeObject("oa", "object");
        proxies[1]->deactivateObjectAdapter("oa");
    }
    cout << "ok" << endl;

    cout << "testing object migration..." << flush;
    {
        proxies[0]->activateObjectAdapter("oa", "oa1", "");
        proxies[1]->activateObjectAdapter("oa", "oa2", "");

        proxies[0]->addObject("oa", "object");
        communicator->stringToProxy("object @ oa1")->ice_ping();
        communicator->stringToProxy("object")->ice_ping();
        proxies[0]->removeObject("oa", "object");

        proxies[1]->addObject("oa", "object");
        communicator->stringToProxy("object @ oa2")->ice_ping();
        communicator->stringToProxy("object")->ice_ping();
        proxies[1]->removeObject("oa", "object");

        try
        {
            communicator->stringToProxy("object @ oa1")->ice_ping();
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            communicator->stringToProxy("object @ oa2")->ice_ping();
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        proxies[0]->deactivateObjectAdapter("oa");
        proxies[1]->deactivateObjectAdapter("oa");
    }
    cout << "ok" << endl;

    cout << "testing replica groups..." << flush;
    {
        proxies[0]->activateObjectAdapter("oa", "oa1", "rg");
        proxies[1]->activateObjectAdapter("oa", "oa2", "rg");
        proxies[2]->activateObjectAdapter("oa", "oa3", "rg");

        proxies[0]->addObject("oa", "object");
        proxies[1]->addObject("oa", "object");
        proxies[2]->addObject("oa", "object");

        communicator->stringToProxy("object @ oa1")->ice_ping();
        communicator->stringToProxy("object @ oa2")->ice_ping();
        communicator->stringToProxy("object @ oa3")->ice_ping();

        communicator->stringToProxy("object @ rg")->ice_ping();

        set<string> adapterIds;
        adapterIds.insert("oa1");
        adapterIds.insert("oa2");
        adapterIds.insert("oa3");
        TestIntfPrx intf = TestIntfPrx::uncheckedCast(communicator->stringToProxy("object"));
        intf = intf->ice_connectionCached(false)->ice_locatorCacheTimeout(0);
        while(!adapterIds.empty())
        {
            adapterIds.erase(intf->getAdapterId());
        }

        while(true)
        {
            adapterIds.insert("oa1");
            adapterIds.insert("oa2");
            adapterIds.insert("oa3");
            intf = TestIntfPrx::uncheckedCast(communicator->stringToProxy("object @ rg"))->ice_connectionCached(false);
            int nRetry = 100;
            while(!adapterIds.empty() && --nRetry > 0)
            {
                adapterIds.erase(intf->getAdapterId());
            }
            if(nRetry > 0)
            {
                break;
            }

            // The previous locator lookup probably didn't return all the replicas... try again.
            communicator->stringToProxy("object @ rg")->ice_locatorCacheTimeout(0)->ice_ping();
        }

        proxies[0]->deactivateObjectAdapter("oa");
        proxies[1]->deactivateObjectAdapter("oa");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("object @ rg"))->getAdapterId() == "oa3");
        proxies[2]->deactivateObjectAdapter("oa");

        proxies[0]->activateObjectAdapter("oa", "oa1", "rg");
        proxies[0]->addObject("oa", "object");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("object @ rg"))->getAdapterId() == "oa1");
        proxies[0]->deactivateObjectAdapter("oa");
    }
    cout << "ok" << endl;

    cout << "shutting down... " << flush;
    for(vector<ControllerPrx>::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
        (*p)->shutdown();
    }
    cout << "ok" << endl;
}
