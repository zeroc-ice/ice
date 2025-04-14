// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceGrid/IceGrid.h"
#include "IceLocatorDiscovery/IceLocatorDiscovery.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();

    TestIntfPrx obj(communicator, "test @ TestAdapter");
    auto locator = Ice::uncheckedCast<IceGrid::LocatorPrx>(obj);

    cout << "pinging server... " << flush;
    locator->ice_ping();
    cout << "ok" << endl;

    cout << "testing locator finder... " << flush;
    auto finder =
        communicator->getDefaultLocator()->ice_identity<Ice::LocatorFinderPrx>(Ice::Identity{"LocatorFinder", "Ice"});
    test(finder->getLocator());
    cout << "ok" << endl;

    Ice::CommunicatorPtr com;

    cout << "testing discovery... " << flush;
    {
        // Add test well-known object
        IceGrid::RegistryPrx registry(
            communicator,
            communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry");

        optional<IceGrid::AdminSessionPrx> session = registry->createAdminSession("foo", "bar");
        session->getAdmin()->addObjectWithType(locator, "::Test");
        session->destroy();

        //
        // Ensure the IceGrid discovery locator can discover the
        // registries and make sure locator requests are forwarded.
        //
        Ice::InitializationData initData;

        bool setPluginProperty = true;
        if (IceInternal::isMinBuild())
        {
            initData.pluginFactories = {
                Ice::udpPluginFactory(),
                Ice::wsPluginFactory(),
                IceLocatorDiscovery::locatorDiscoveryPluginFactory()};

            setPluginProperty = false;
        }

        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Default.Locator", "");
        if (setPluginProperty)
        {
            initData.properties->setProperty(
                "Ice.Plugin.IceLocatorDiscovery",
                "IceLocatorDiscovery:createIceLocatorDiscovery");
        }
        initData.properties->setProperty("AdapterForDiscoveryTest.AdapterId", "discoveryAdapter");
        initData.properties->setProperty("AdapterForDiscoveryTest.Endpoints", "default");

        com = Ice::initialize(initData);
        test(com->getDefaultLocator());

        com->stringToProxy("test @ TestAdapter")->ice_ping();
        com->stringToProxy("test")->ice_ping();
        test(com->getDefaultLocator()->getRegistry());
        test(Ice::uncheckedCast<IceGrid::LocatorPrx>(com->getDefaultLocator().value())->getLocalRegistry());
        test(Ice::uncheckedCast<IceGrid::LocatorPrx>(com->getDefaultLocator().value())->getLocalQuery());

        Ice::ObjectAdapterPtr adapter = com->createObjectAdapter("AdapterForDiscoveryTest");
        adapter->activate();
        adapter->deactivate();

        com->destroy();

        // Now, ensure that the IceGrid discovery locator correctly handles failure to find a locator.

        initData.properties->setProperty("IceLocatorDiscovery.InstanceName", "unknown");
        initData.properties->setProperty("IceLocatorDiscovery.RetryCount", "1");
        initData.properties->setProperty("IceLocatorDiscovery.Timeout", "100");

        com = Ice::initialize(initData);
        test(com->getDefaultLocator());
        try
        {
            com->stringToProxy("test @ TestAdapter")->ice_ping();
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        try
        {
            com->stringToProxy("test")->ice_ping();
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        test(!com->getDefaultLocator()->getRegistry());
        test(!Ice::checkedCast<IceGrid::LocatorPrx>(com->getDefaultLocator()));
        try
        {
            test(Ice::uncheckedCast<IceGrid::LocatorPrx>(com->getDefaultLocator())->getLocalQuery());
        }
        catch (const Ice::OperationNotExistException&)
        {
        }

        adapter = com->createObjectAdapter("AdapterForDiscoveryTest");
        adapter->activate();
        adapter->deactivate();

        com->destroy();

        string multicast;
        if (communicator->getProperties()->getIceProperty("Ice.IPv6") == "1")
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
        if (setPluginProperty)
        {
            initData.properties->setProperty(
                "Ice.Plugin.IceLocatorDiscovery",
                "IceLocatorDiscovery:createIceLocatorDiscovery");
        }
        initData.properties->setProperty("IceLocatorDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");
        com = Ice::initialize(initData);
        test(com->getDefaultLocator());
        try
        {
            com->stringToProxy("test @ TestAdapter")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        com->destroy();

        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Default.Locator", "");
        initData.properties->setProperty("IceLocatorDiscovery.RetryCount", "0");
        if (setPluginProperty)
        {
            initData.properties->setProperty(
                "Ice.Plugin.IceLocatorDiscovery",
                "IceLocatorDiscovery:createIceLocatorDiscovery");
        }
        initData.properties->setProperty("IceLocatorDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");
        com = Ice::initialize(initData);
        test(com->getDefaultLocator());
        try
        {
            com->stringToProxy("test @ TestAdapter")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        com->destroy();

        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.Default.Locator", "");
        initData.properties->setProperty("IceLocatorDiscovery.RetryCount", "1");
        if (setPluginProperty)
        {
            initData.properties->setProperty(
                "Ice.Plugin.IceLocatorDiscovery",
                "IceLocatorDiscovery:createIceLocatorDiscovery");
        }
        {
            string intf = initData.properties->getIceProperty("IceLocatorDiscovery.Interface");
            if (!intf.empty())
            {
                intf = " --interface \"" + intf + "\"";
            }
            ostringstream port;
            port << TestHelper::getTestPort(initData.properties, 99);
            initData.properties->setProperty(
                "IceLocatorDiscovery.Lookup",
                "udp -h " + multicast + " --interface unknown:" + "udp -h " + multicast + " -p " + port.str() + intf);
        }
        com = Ice::initialize(initData);
        test(com->getDefaultLocator());
        try
        {
            com->stringToProxy("test @ TestAdapter")->ice_ping();
        }
        catch (const Ice::NoEndpointException&)
        {
            test(false);
        }
        com->destroy();
    }
    cout << "ok" << endl;

    cout << "shutting down server... " << flush;
    obj->shutdown();
    cout << "ok" << endl;
}

void
allTestsWithDeploy(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();

    TestIntfPrx obj(communicator, "test @ TestAdapter");
    TestIntfPrx obj2(communicator, "test");

    cout << "pinging server... " << flush;
    obj->ice_ping();
    obj2->ice_ping();
    cout << "ok" << endl;

    cout << "testing encoding versioning... " << flush;
    Ice::ObjectPrx base10(communicator, "test10 @ TestAdapter10");
    Ice::ObjectPrx base102(communicator, "test10");

    try
    {
        base10->ice_ping();
        test(false);
    }
    catch (const Ice::NoEndpointException&)
    {
    }
    try
    {
        base102->ice_ping();
        test(false);
    }
    catch (const Ice::NoEndpointException&)
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
        Ice::ObjectPrx(communicator, "unknown/unknown")->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject() == "object");
        test(ex.id() == "unknown/unknown");
    }
    cout << "ok" << endl;

    cout << "testing reference with unknown adapter... " << flush;
    try
    {
        Ice::ObjectPrx(communicator, "test @ TestAdapterUnknown")->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject() == "object adapter");
        test(ex.id() == "TestAdapterUnknown");
    }
    cout << "ok" << endl;

    IceGrid::RegistryPrx registry(
        communicator,
        communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry");

    optional<IceGrid::AdminSessionPrx> session = registry->createAdminSession("foo", "bar");
    optional<IceGrid::AdminPrx> admin = session->getAdmin();
    admin->enableServer("server", false);
    admin->stopServer("server");

    cout << "testing whether server is still reachable... " << flush;
    try
    {
        obj->ice_ping();
        test(false);
    }
    catch (const Ice::NoEndpointException&)
    {
    }
    try
    {
        obj2->ice_ping();
        test(false);
    }
    catch (const Ice::NoEndpointException&)
    {
    }

    admin->enableServer("server", true);

    try
    {
        obj->ice_ping();
    }
    catch (const Ice::NoEndpointException&)
    {
        test(false);
    }
    try
    {
        obj2->ice_ping();
    }
    catch (const Ice::NoEndpointException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    admin->stopServer("server");

    session->destroy();
}
