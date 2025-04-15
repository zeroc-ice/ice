// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceDiscovery/IceDiscovery.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

void
allTests(Test::TestHelper* helper, int num)
{
    CommunicatorPtr communicator = helper->communicator();
    vector<optional<ControllerPrx>> proxies;
    vector<optional<ControllerPrx>> indirectProxies;
    for (int i = 0; i < num; ++i)
    {
        {
            ostringstream os;
            os << "controller" << i;
            proxies.emplace_back(ControllerPrx(communicator, os.str()));
        }
        {
            ostringstream os;
            os << "controller" << i << "@control" << i;
            indirectProxies.emplace_back(ControllerPrx(communicator, os.str()));
        }
    }

    cout << "testing indirect proxies... " << flush;
    {
        for (const auto& indirectProxy : indirectProxies)
        {
            indirectProxy->ice_ping();
        }
    }
    cout << "ok" << endl;

    cout << "testing well-known proxies... " << flush;
    {
        for (const auto& proxy : proxies)
        {
            proxy->ice_ping();
        }
    }
    cout << "ok" << endl;

    cout << "testing object adapter registration... " << flush;
    {
        try
        {
            ObjectPrx(communicator, "object @ oa1")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }

        proxies[0]->activateObjectAdapter("oa", "oa1", "");

        try
        {
            ObjectPrx(communicator, "object @ oa1")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        proxies[0]->deactivateObjectAdapter("oa");

        try
        {
            ObjectPrx(communicator, "object @ oa1")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing object adapter migration..." << flush;
    {
        proxies[0]->activateObjectAdapter("oa", "oa1", "");
        proxies[0]->addObject("oa", "object");
        ObjectPrx(communicator, "object @ oa1")->ice_ping();
        proxies[0]->removeObject("oa", "object");
        proxies[0]->deactivateObjectAdapter("oa");

        proxies[1]->activateObjectAdapter("oa", "oa1", "");
        proxies[1]->addObject("oa", "object");
        ObjectPrx(communicator, "object @ oa1")->ice_ping();
        proxies[1]->removeObject("oa", "object");
        proxies[1]->deactivateObjectAdapter("oa");
    }
    cout << "ok" << endl;

    cout << "testing object migration..." << flush;
    {
        proxies[0]->activateObjectAdapter("oa", "oa1", "");
        proxies[1]->activateObjectAdapter("oa", "oa2", "");

        proxies[0]->addObject("oa", "object");
        ObjectPrx(communicator, "object @ oa1")->ice_ping();
        ObjectPrx(communicator, "object")->ice_ping();
        proxies[0]->removeObject("oa", "object");

        proxies[1]->addObject("oa", "object");
        ObjectPrx(communicator, "object @ oa2")->ice_ping();
        ObjectPrx(communicator, "object")->ice_ping();
        proxies[1]->removeObject("oa", "object");

        try
        {
            ObjectPrx(communicator, "object @ oa1")->ice_ping();
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            ObjectPrx(communicator, "object @ oa2")->ice_ping();
        }
        catch (const Ice::ObjectNotExistException&)
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

        ObjectPrx(communicator, "object @ oa1")->ice_ping();
        ObjectPrx(communicator, "object @ oa2")->ice_ping();
        ObjectPrx(communicator, "object @ oa3")->ice_ping();

        ObjectPrx(communicator, "object @ rg")->ice_ping();

        set<string> adapterIds;
        adapterIds.insert("oa1");
        adapterIds.insert("oa2");
        adapterIds.insert("oa3");
        TestIntfPrx intf(communicator, "object");
        intf = intf->ice_connectionCached(false)->ice_locatorCacheTimeout(0);
        while (!adapterIds.empty())
        {
            adapterIds.erase(intf->getAdapterId());
        }

        while (true)
        {
            adapterIds.insert("oa1");
            adapterIds.insert("oa2");
            adapterIds.insert("oa3");
            intf = TestIntfPrx(communicator, "object @ rg")->ice_connectionCached(false);
            int nRetry = 100;
            while (!adapterIds.empty() && --nRetry > 0)
            {
                adapterIds.erase(intf->getAdapterId());
            }
            if (nRetry > 0)
            {
                break;
            }

            // The previous locator lookup probably didn't return all the replicas... try again.
            ObjectPrx(communicator, "object @ rg")->ice_locatorCacheTimeout(0)->ice_ping();
        }

        proxies[0]->deactivateObjectAdapter("oa");
        proxies[1]->deactivateObjectAdapter("oa");
        test(TestIntfPrx(communicator, "object @ rg")->getAdapterId() == "oa3");
        proxies[2]->deactivateObjectAdapter("oa");

        proxies[0]->activateObjectAdapter("oa", "oa1", "rg");
        proxies[0]->addObject("oa", "object");
        test(TestIntfPrx(communicator, "object @ rg")->getAdapterId() == "oa1");
        proxies[0]->deactivateObjectAdapter("oa");
    }
    cout << "ok" << endl;

    cout << "testing invalid lookup endpoints... " << flush;
    {
        string multicast;
        if (communicator->getProperties()->getIceProperty("Ice.IPv6") == "1")
        {
            multicast = "\"ff15::1\"";
        }
        else
        {
            multicast = "239.255.0.1";
        }

        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            initData.properties->setProperty("IceDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");

            if (IceInternal::isMinBuild())
            {
                initData.pluginFactories = {
                    Ice::udpPluginFactory(),
                    Ice::wsPluginFactory(),
                    IceDiscovery::discoveryPluginFactory()};
            }

            Ice::CommunicatorPtr com = Ice::initialize(initData);
            test(com->getDefaultLocator());
            try
            {
                ObjectPrx(com, "controller0@control0")->ice_ping();
                test(false);
            }
            catch (const Ice::LocalException&)
            {
            }
            com->destroy();
        }
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            string intf = initData.properties->getIceProperty("IceDiscovery.Interface");
            if (!intf.empty())
            {
                intf = " --interface \"" + intf + "\"";
            }
            string port = initData.properties->getIceProperty("IceDiscovery.Port");
            initData.properties->setProperty(
                "IceDiscovery.Lookup",
                "udp -h " + multicast + " --interface unknown:" + "udp -h " + multicast + " -p " + port + intf);

            if (IceInternal::isMinBuild())
            {
                initData.pluginFactories = {
                    Ice::udpPluginFactory(),
                    Ice::wsPluginFactory(),
                    IceDiscovery::discoveryPluginFactory()};
            }

            Ice::CommunicatorPtr com = Ice::initialize(initData);
            test(com->getDefaultLocator());
            ObjectPrx(com, "controller0@control0")->ice_ping();
            com->destroy();
        }
    }
    cout << "ok" << endl;

    cout << "shutting down... " << flush;
    for (const auto& proxy : proxies)
    {
        proxy->shutdown();
    }
    cout << "ok" << endl;
}
