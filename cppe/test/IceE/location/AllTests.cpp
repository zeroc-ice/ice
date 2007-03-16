// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestCommon.h>
#include <Test.h>

#ifdef ICEE_HAS_ROUTER
#   include <IceE/Router.h>
#endif

using namespace std;
using namespace Test;

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    ServerManagerPrx manager = ServerManagerPrx::checkedCast(
	communicator->stringToProxy(
	    communicator->getProperties()->getPropertyWithDefault(
		"Location.Proxy", "ServerManager:default -p 12010 -t 10000")));
    Ice::LocatorPrx locator = Ice::LocatorPrx::uncheckedCast(communicator->getDefaultLocator());
    test(manager);

    tprintf("testing stringToProxy...");
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    Ice::ObjectPrx base2 = communicator->stringToProxy("test @ TestAdapter");
    Ice::ObjectPrx base3 = communicator->stringToProxy("test");
    Ice::ObjectPrx base4 = communicator->stringToProxy("ServerManager"); 
    Ice::ObjectPrx base5 = communicator->stringToProxy("test2");
    Ice::ObjectPrx base6 = communicator->stringToProxy("test @ ReplicatedAdapter");
    tprintf("ok\n");

    tprintf("testing ice_locator and ice_getLocator... ");
    test(Ice::proxyIdentityEqual(base->ice_getLocator(), communicator->getDefaultLocator()));
    Ice::LocatorPrx anotherLocator = Ice::LocatorPrx::uncheckedCast(communicator->stringToProxy("anotherLocator"));
    base = base->ice_locator(anotherLocator);
    test(Ice::proxyIdentityEqual(base->ice_getLocator(), anotherLocator));
    communicator->setDefaultLocator(0);
    base = communicator->stringToProxy("test @ TestAdapter");
    test(!base->ice_getLocator());
    base = base->ice_locator(anotherLocator);
    test(Ice::proxyIdentityEqual(base->ice_getLocator(), anotherLocator));
    communicator->setDefaultLocator(locator);
    base = communicator->stringToProxy("test @ TestAdapter");
    test(Ice::proxyIdentityEqual(base->ice_getLocator(), communicator->getDefaultLocator())); 
    
#ifdef ICEE_HAS_ROUTER
    //
    // We also test ice_router/ice_getRouter (perhaps we should add a
    // test/Ice/router test?)
    //
    test(!base->ice_getRouter());
    Ice::RouterPrx anotherRouter = Ice::RouterPrx::uncheckedCast(communicator->stringToProxy("anotherRouter"));
    base = base->ice_router(anotherRouter);
    test(Ice::proxyIdentityEqual(base->ice_getRouter(), anotherRouter));
    Ice::RouterPrx router = Ice::RouterPrx::uncheckedCast(communicator->stringToProxy("dummyrouter"));
    communicator->setDefaultRouter(router);
    base = communicator->stringToProxy("test @ TestAdapter");
    test(Ice::proxyIdentityEqual(base->ice_getRouter(), communicator->getDefaultRouter()));
    communicator->setDefaultRouter(0);
    base = communicator->stringToProxy("test @ TestAdapter");
    test(!base->ice_getRouter());
#endif
    tprintf("ok\n");
    
    tprintf("starting server...");
    manager->startServer();
    tprintf("ok\n");

    tprintf("testing checked cast...");
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("test@TestAdapter"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("test   @TestAdapter"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("test@   TestAdapter"));
    test(obj);
    TestIntfPrx obj2 = TestIntfPrx::checkedCast(base2);
    test(obj2);
    TestIntfPrx obj3 = TestIntfPrx::checkedCast(base3);
    test(obj3);
    ServerManagerPrx obj4 = ServerManagerPrx::checkedCast(base4);
    test(obj4);
    TestIntfPrx obj5 = TestIntfPrx::checkedCast(base5);
    test(obj5);
    TestIntfPrx obj6 = TestIntfPrx::checkedCast(base6);
    test(obj6);
    tprintf("ok\n");
 
    tprintf("testing id@AdapterId indirect proxy...");
    obj->shutdown();
    manager->startServer();
    try
    {
	obj2 = TestIntfPrx::checkedCast(base2);
	obj2->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    tprintf("ok\n");

    tprintf("testing id@ReplicaGroupId indirect proxy...");
    obj->shutdown();
    manager->startServer();
    try
    {
	obj6 = TestIntfPrx::checkedCast(base6);
	obj6->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    tprintf("ok\n");
    
    tprintf("testing identity indirect proxy...");
    obj->shutdown();
    manager->startServer();
    try
    {
	obj3 = TestIntfPrx::checkedCast(base3);
	obj3->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    try
    {
	obj2 = TestIntfPrx::checkedCast(base2);
	obj2->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
	obj2 = TestIntfPrx::checkedCast(base2);
	obj2->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    try
    {
	obj3 = TestIntfPrx::checkedCast(base3);
	obj3->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    obj->shutdown();
    manager->startServer();

    try
    {
	obj2 = TestIntfPrx::checkedCast(base2);
	obj2->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
	obj3 = TestIntfPrx::checkedCast(base3);
	obj3->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
	obj2 = TestIntfPrx::checkedCast(base2);
	obj2->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    obj->shutdown();
    manager->startServer();

    try
    {
	obj5 = TestIntfPrx::checkedCast(base5);
	obj5->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    tprintf("ok\n");

    tprintf("testing reference with unknown identity...");
    try
    {
	base = communicator->stringToProxy("unknown/unknown");
	base->ice_ping();
	test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
	test(ex.kindOfObject == "object");
	test(ex.id == "unknown/unknown");
    }
    tprintf("ok\n");

    tprintf("testing reference with unknown adapter...");
    try
    {
	base = communicator->stringToProxy("test @ TestAdapterUnknown");
	base->ice_ping();
	test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
	test(ex.kindOfObject == "object adapter");
	test(ex.id == "TestAdapterUnknown");
    }
    tprintf("ok\n");

    tprintf("testing object reference from server...");
    HelloPrx hello = obj->getHello();
    hello->sayHello();
    test(communicator->proxyToString(hello).find("TestAdapter") != string::npos);
    hello = obj->getReplicatedHello();
    hello->sayHello();
    test(communicator->proxyToString(hello).find("ReplicatedAdapter") != string::npos);
    tprintf("ok\n");

    tprintf("testing object reference from server after shutdown...");
    obj->shutdown();
    manager->startServer();
    hello->sayHello();
    tprintf("ok\n");

    tprintf("testing object migration...");
    hello = HelloPrx::checkedCast(communicator->stringToProxy("hello"));
    obj->migrateHello();
    hello->sayHello();
    obj->migrateHello();
    hello->sayHello();
    obj->migrateHello();
    hello->sayHello();
    tprintf("ok\n");

    tprintf("shutdown server...");
    obj->shutdown();
    tprintf("ok\n");

    tprintf("testing whether server is gone...");
    try
    {
	obj2->ice_ping();
	test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    try
    {
	obj3->ice_ping();
	test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    try
    {
	obj5->ice_ping();
	test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    tprintf("ok\n");

    tprintf("shutdown server manager...");
    manager->shutdown();
    tprintf("ok\n");
}

#endif
