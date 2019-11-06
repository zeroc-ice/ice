//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace IceGrid;
using namespace Test;

void
allTests(TestHelper* helper)
{
    auto communicator = helper->communicator();
    cout << "testing stringToProxy... " << flush;
    auto base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    cout << "ok" << endl;

    cout << "testing IceGrid.Locator is present... " << flush;
    auto locator = Ice::uncheckedCast<LocatorPrx>(base);
    test(locator);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    auto obj = Ice::checkedCast<TestIntfPrx>(base);
    test(obj);
    test(Ice::targetEqualTo(obj, base));
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    cout << "ok" << endl;

    cout << "testing locator finder... " << flush;
    Ice::Identity finderId{ "Ice", "LocatorFinder" };
    auto finder = Ice::checkedCast<Ice::LocatorFinderPrx>(communicator->getDefaultLocator()->ice_identity(finderId));
    test(finder->getLocator());
    cout << "ok" << endl;

    shared_ptr<Ice::Communicator> com;
    try
    {
        cout << "testing discovery... " << flush;
        {
            // Add test well-known object
            auto registry = Ice::checkedCast<RegistryPrx>(communicator->stringToProxy(
                communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
            test(registry);

            auto session = registry->createAdminSession("foo", "bar");
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
            test(Ice::checkedCast<LocatorPrx>(com->getDefaultLocator()));
            test(Ice::uncheckedCast<LocatorPrx>(com->getDefaultLocator())->getLocalRegistry());
            test(Ice::uncheckedCast<LocatorPrx>(com->getDefaultLocator())->getLocalQuery());

            auto adapter = com->createObjectAdapter("AdapterForDiscoveryTest");
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
            test(!Ice::checkedCast<LocatorPrx>(com->getDefaultLocator()));
            try
            {
                test(Ice::uncheckedCast<LocatorPrx>(com->getDefaultLocator())->getLocalQuery());
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
            initData.properties->setProperty("IceLocatorDiscovery.RetryCount", "0");
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
            initData.properties->setProperty("IceLocatorDiscovery.RetryCount", "1");
            initData.properties->setProperty("Ice.Plugin.IceLocatorDiscovery",
                                             "IceLocatorDiscovery:createIceLocatorDiscovery");
            {
                string intf = initData.properties->getProperty("IceLocatorDiscovery.Interface");
                if(!intf.empty())
                {
                    intf = " --interface \"" + intf + "\"";
                }
                ostringstream port;
                port << TestHelper::getTestPort(initData.properties, 99);
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
allTestsWithDeploy(TestHelper* helper)
{
    auto communicator = helper->communicator();
    cout << "testing stringToProxy... " << flush;
    auto base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    auto base2 = communicator->stringToProxy("test");
    test(base2);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    auto obj = Ice::checkedCast<TestIntfPrx>(base);
    test(obj);
    test(Ice::targetEqualTo(obj, base));
    auto obj2 = Ice::checkedCast<TestIntfPrx>(base2);
    test(obj2);
    test(Ice::targetEqualTo(obj2, base2));
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    obj2->ice_ping();
    cout << "ok" << endl;

    cout << "testing encoding versioning... " << flush;
    auto base10 = communicator->stringToProxy("test10 @ TestAdapter10");
    test(base10);
    auto base102 = communicator->stringToProxy("test10");
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

    auto registry = Ice::checkedCast<RegistryPrx>(communicator->stringToProxy(
        communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);

    auto session = registry->createAdminSession("foo", "bar");

    session->ice_getConnection()->setACM(registry->getACMTimeout(), Ice::nullopt, Ice::ACMHeartbeat::HeartbeatAlways);

    auto admin = session->getAdmin();
    test(admin);

    admin->enableServer("server", false);
    admin->stopServer("server");

    cout << "testing whether server is still reachable... " << flush;
    try
    {
        obj = Ice::checkedCast<TestIntfPrx>(base);
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    try
    {
        obj2 = Ice::checkedCast<TestIntfPrx>(base2);
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }

    admin->enableServer("server", true);

    try
    {
        obj = Ice::checkedCast<TestIntfPrx>(base);
    }
    catch(const Ice::NoEndpointException&)
    {
        test(false);
    }
    try
    {
        obj2 = Ice::checkedCast<TestIntfPrx>(base2);
    }
    catch(const Ice::NoEndpointException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    admin->stopServer("server");

    session->destroy();
}
