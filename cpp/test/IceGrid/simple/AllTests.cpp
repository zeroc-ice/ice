// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrxPtr base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    cout << "ok" << endl;

    cout << "testing IceGrid.Locator is present... " << flush;
    IceGrid::LocatorPrxPtr locator = ICE_UNCHECKED_CAST(IceGrid::LocatorPrx, base);
    test(locator);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrxPtr obj = ICE_CHECKED_CAST(TestIntfPrx, base);
    test(obj);
    test(Ice::targetEqualTo(obj, base));
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    cout << "ok" << endl;

    cout << "testing locator finder... " << flush;
    Ice::Identity finderId;
    finderId.category = "Ice";
    finderId.name = "LocatorFinder";
    Ice::LocatorFinderPrxPtr finder = ICE_CHECKED_CAST(Ice::LocatorFinderPrx,
        communicator->getDefaultLocator()->ice_identity(finderId));
    test(finder->getLocator());
    cout << "ok" << endl;

    Ice::CommunicatorPtr com;
    try
    {
        cout << "testing discovery... " << flush;
        {
            // Add test well-known object
            IceGrid::RegistryPrxPtr registry = ICE_CHECKED_CAST(IceGrid::RegistryPrx,
                communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
            test(registry);

            IceGrid::AdminSessionPrxPtr session = registry->createAdminSession("foo", "bar");
            session->getAdmin()->addObjectWithType(base, "::Test");
            session->destroy();

            //
            // Ensure the IceGrid discovery locator can discover the
            // registries and make sure locator requests are forwarded.
            //
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            initData.properties->setProperty("Ice.Default.Locator", "");
            initData.properties->setProperty("Ice.Plugin.IceLocatorDiscovery",
                                             "IceLocatorDiscovery:createIceLocatorDiscovery");
            initData.properties->setProperty("AdapterForDiscoveryTest.AdapterId", "discoveryAdapter");
            initData.properties->setProperty("AdapterForDiscoveryTest.Endpoints", "default");

            com = Ice::initialize(initData);
            test(com->getDefaultLocator());

            com->stringToProxy("test @ TestAdapter")->ice_ping();
            com->stringToProxy("test")->ice_ping();
            test(com->getDefaultLocator()->getRegistry());
            test(ICE_CHECKED_CAST(IceGrid::LocatorPrx, com->getDefaultLocator()));
            test(ICE_UNCHECKED_CAST(IceGrid::LocatorPrx, com->getDefaultLocator())->getLocalRegistry());
            test(ICE_UNCHECKED_CAST(IceGrid::LocatorPrx, com->getDefaultLocator())->getLocalQuery());

            Ice::ObjectAdapterPtr adapter = com->createObjectAdapter("AdapterForDiscoveryTest");
            adapter->activate();
            adapter->deactivate();

            com->destroy();

            //
            // Now, ensure that the IceGrid discovery locator correctly
            // handles failure to find a locator. Also test
            // Ice::registerIceLocatorDiscovery()
            //
#ifndef ICE_STATIC_LIBS
            Ice::registerIceLocatorDiscovery();
            initData.properties->setProperty("Ice.Plugin.IceLocatorDiscovery", "");
#else
            initData.properties->setProperty("Ice.Plugin.IceLocatorDiscovery", "1");
#endif
            initData.properties->setProperty("IceLocatorDiscovery.InstanceName", "unknown");
            initData.properties->setProperty("IceLocatorDiscovery.RetryCount", "1");
            initData.properties->setProperty("IceLocatorDiscovery.Timeout", "100");
            com = Ice::initialize(initData);
            test(com->getDefaultLocator());
            try
            {
                com->stringToProxy("test @ TestAdapter")->ice_ping();
            }
            catch(const Ice::NoEndpointException&)
            {
            }
            try
            {
                com->stringToProxy("test")->ice_ping();
            }
            catch(const Ice::NoEndpointException&)
            {
            }
            test(!com->getDefaultLocator()->getRegistry());
            test(!ICE_CHECKED_CAST(IceGrid::LocatorPrx, com->getDefaultLocator()));
            try
            {
                test(ICE_UNCHECKED_CAST(IceGrid::LocatorPrx, com->getDefaultLocator())->getLocalQuery());
            }
            catch(const Ice::OperationNotExistException&)
            {
            }

            adapter = com->createObjectAdapter("AdapterForDiscoveryTest");
            adapter->activate();
            adapter->deactivate();

            com->destroy();

            string multicast;
            if(communicator->getProperties()->getProperty("Ice.IPv6") == "1")
            {
                multicast = "\"ff15::1\"";
            }
            else
            {
                multicast = "239.255.0.1";
            }

            //
            // Test invalid lookup endpoints
            //
            initData.properties = communicator->getProperties()->clone();
            initData.properties->setProperty("Ice.Default.Locator", "");
            initData.properties->setProperty("Ice.Plugin.IceLocatorDiscovery",
                                             "IceLocatorDiscovery:createIceLocatorDiscovery");
            initData.properties->setProperty("IceLocatorDiscovery.Lookup",
                                             "udp -h " + multicast + " --interface unknown");
            com = Ice::initialize(initData);
            test(com->getDefaultLocator());
            try
            {
                com->stringToProxy("test @ TestAdapter")->ice_ping();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
            com->destroy();

            initData.properties = communicator->getProperties()->clone();
            initData.properties->setProperty("Ice.Default.Locator", "");
            initData.properties->setProperty("Ice.Plugin.IceLocatorDiscovery",
                                             "IceLocatorDiscovery:createIceLocatorDiscovery");
            {
                string intf = initData.properties->getProperty("IceLocatorDiscovery.Interface");
                if(!intf.empty())
                {
                    intf = " --interface \"" + intf + "\"";
                }
                ostringstream port;
                port << getTestPort(initData.properties, 99);
                initData.properties->setProperty("IceLocatorDiscovery.Lookup",
                                                 "udp -h " + multicast + " --interface unknown:" +
                                                 "udp -h " + multicast + " -p " + port.str() + intf);
            }
            com = Ice::initialize(initData);
            test(com->getDefaultLocator());
            try
            {
                com->stringToProxy("test @ TestAdapter")->ice_ping();
            }
            catch(const Ice::NoEndpointException&)
            {
                test(false);
            }
            com->destroy();
        }
        cout << "ok" << endl;
    }
    catch(const Ice::NoEndpointException&)
    {
        com->destroy();
        cout << "failed (is a firewall enabled?)" << endl;
    }
    cout << "shutting down server... " << flush;
    obj->shutdown();
    cout << "ok" << endl;
}

void
allTestsWithDeploy(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrxPtr base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    Ice::ObjectPrxPtr base2 = communicator->stringToProxy("test");
    test(base2);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrxPtr obj = ICE_CHECKED_CAST(TestIntfPrx, base);
    test(obj);
    test(Ice::targetEqualTo(obj, base));
    TestIntfPrxPtr obj2 = ICE_CHECKED_CAST(TestIntfPrx, base2);
    test(obj2);
    test(Ice::targetEqualTo(obj2, base2));
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    obj2->ice_ping();
    cout << "ok" << endl;

    cout << "testing encoding versioning... " << flush;
    Ice::ObjectPrxPtr base10 = communicator->stringToProxy("test10 @ TestAdapter10");
    test(base10);
    Ice::ObjectPrxPtr base102 = communicator->stringToProxy("test10");
    test(base102);
    try
    {
        base10->ice_ping();
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    try
    {
        base102->ice_ping();
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    base10 = base10->ice_encodingVersion(Ice::Encoding_1_0);
    base102 = base102->ice_encodingVersion(Ice::Encoding_1_0);
    base10->ice_ping();
    base102->ice_ping();
    cout << "ok" << endl;

    cout << "testing reference with unknown identity... " << flush;
    try
    {
        communicator->stringToProxy("unknown/unknown")->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject == "object");
        test(ex.id == "unknown/unknown");
    }
    cout << "ok" << endl;

    cout << "testing reference with unknown adapter... " << flush;
    try
    {
        communicator->stringToProxy("test @ TestAdapterUnknown")->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject == "object adapter");
        test(ex.id == "TestAdapterUnknown");
    }
    cout << "ok" << endl;

    IceGrid::RegistryPrxPtr registry = ICE_CHECKED_CAST(IceGrid::RegistryPrx,
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);

    IceGrid::AdminSessionPrxPtr session = registry->createAdminSession("foo", "bar");

    session->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));

    IceGrid::AdminPrxPtr admin = session->getAdmin();
    test(admin);

    admin->enableServer("server", false);
    admin->stopServer("server");

    cout << "testing whether server is still reachable... " << flush;
    try
    {
        obj = ICE_CHECKED_CAST(TestIntfPrx, base);
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    try
    {
        obj2 = ICE_CHECKED_CAST(TestIntfPrx, base2);
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }

    admin->enableServer("server", true);

    try
    {
        obj = ICE_CHECKED_CAST(TestIntfPrx, base);
    }
    catch(const Ice::NoEndpointException&)
    {
        test(false);
    }
    try
    {
        obj2 = ICE_CHECKED_CAST(TestIntfPrx, base2);
    }
    catch(const Ice::NoEndpointException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    admin->stopServer("server");

    session->destroy();
}
