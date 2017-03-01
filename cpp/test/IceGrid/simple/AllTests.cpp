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
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    cout << "ok" << endl;

    cout << "testing IceGrid.Locator is present... " << flush;
    IceGrid::LocatorPrx locator = IceGrid::LocatorPrx::uncheckedCast(base);
    test(locator);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    cout << "ok" << endl;

    cout << "testing locator finder... " << flush;
    Ice::Identity finderId;
    finderId.category = "Ice";
    finderId.name = "LocatorFinder";
    Ice::LocatorFinderPrx finder = Ice::LocatorFinderPrx::checkedCast(
        communicator->getDefaultLocator()->ice_identity(finderId));
    test(finder->getLocator());
    cout << "ok" << endl;

    Ice::CommunicatorPtr com;
    try
    {
        cout << "testing discovery... " << flush;
        {
            // Add test well-known object
            IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
                communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
            test(registry);

            IceGrid::AdminSessionPrx session = registry->createAdminSession("foo", "bar");
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
            {
                ostringstream port;
                port << getTestPort(initData.properties, 99);
                initData.properties->setProperty("IceLocatorDiscovery.Port", port.str());
            }
            initData.properties->setProperty("AdapterForDiscoveryTest.AdapterId", "discoveryAdapter");
            initData.properties->setProperty("AdapterForDiscoveryTest.Endpoints", "default");

            com = Ice::initialize(initData);
            test(com->getDefaultLocator());

            com->stringToProxy("test @ TestAdapter")->ice_ping();
            com->stringToProxy("test")->ice_ping();
            test(com->getDefaultLocator()->getRegistry());
            test(IceGrid::LocatorPrx::checkedCast(com->getDefaultLocator()));
            test(IceGrid::LocatorPrx::uncheckedCast(com->getDefaultLocator())->getLocalRegistry());
            test(IceGrid::LocatorPrx::uncheckedCast(com->getDefaultLocator())->getLocalQuery());

            Ice::ObjectAdapterPtr adapter = com->createObjectAdapter("AdapterForDiscoveryTest");
            adapter->activate();
            adapter->deactivate();

            com->destroy();

            //
            // Now, ensure that the IceGrid discovery locator correctly
            // handles failure to find a locator. Also test
            // Ice::registerIceLocatorDiscovery()
            //
            Ice::registerIceLocatorDiscovery();
            initData.properties->setProperty("Ice.Plugin.IceLocatorDiscovery", "");
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
            test(!IceGrid::LocatorPrx::checkedCast(com->getDefaultLocator()));
            try
            {
                test(IceGrid::LocatorPrx::uncheckedCast(com->getDefaultLocator())->getLocalQuery());
            }
            catch(const Ice::OperationNotExistException&)
            {
            }

            adapter = com->createObjectAdapter("AdapterForDiscoveryTest");
            adapter->activate();
            adapter->deactivate();

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
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    Ice::ObjectPrx base2 = communicator->stringToProxy("test");
    test(base2);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    TestIntfPrx obj2 = TestIntfPrx::checkedCast(base2);
    test(obj2);
    test(obj2 == base2);
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    obj2->ice_ping();
    cout << "ok" << endl;

    cout << "testing encoding versioning... " << flush;
    Ice::ObjectPrx base10 = communicator->stringToProxy("test10 @ TestAdapter10");
    test(base10);
    Ice::ObjectPrx base102 = communicator->stringToProxy("test10");
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

    IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);

    IceGrid::AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    session->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));

    IceGrid::AdminPrx admin = session->getAdmin();
    test(admin);

    admin->enableServer("server", false);
    admin->stopServer("server");

    cout << "testing whether server is still reachable... " << flush;
    try
    {
        obj = TestIntfPrx::checkedCast(base);
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }

    admin->enableServer("server", true);

    try
    {
        obj = TestIntfPrx::checkedCast(base);
    }
    catch(const Ice::NoEndpointException&)
    {
        test(false);
    }
    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
    }
    catch(const Ice::NoEndpointException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    admin->stopServer("server");

    session->destroy();
}
