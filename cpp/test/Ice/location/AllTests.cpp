// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Router.h>
#include <IceUtil/IceUtil.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

class HelloI : virtual public Hello
{
public:
    
    virtual void
    sayHello(const Ice::Current& foo)
    {
        // Do nothing, this is just a dummy servant.
    }
};

void
allTests(const Ice::CommunicatorPtr& communicator, const string& ref)
{
    ServerManagerPrx manager = ServerManagerPrx::checkedCast(communicator->stringToProxy(ref));
    TestLocatorPrx locator = TestLocatorPrx::uncheckedCast(communicator->getDefaultLocator());
    test(manager);

    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    Ice::ObjectPrx base2 = communicator->stringToProxy("test @ TestAdapter");
    Ice::ObjectPrx base3 = communicator->stringToProxy("test");
    Ice::ObjectPrx base4 = communicator->stringToProxy("ServerManager"); 
    Ice::ObjectPrx base5 = communicator->stringToProxy("test2");
    Ice::ObjectPrx base6 = communicator->stringToProxy("test @ ReplicatedAdapter");
    cout << "ok" << endl;

    cout << "testing ice_locator and ice_getLocator... " << flush;
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
    cout << "ok" << endl;

    cout << "starting server... " << flush;
    manager->startServer();
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
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
    cout << "ok" << endl;
 
    cout << "testing id@AdapterId indirect proxy... " << flush;
    obj->shutdown();
    manager->startServer();
    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
        obj2->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;    
    
    cout << "testing id@ReplicaGroupId indirect proxy... " << flush;
    obj->shutdown();
    manager->startServer();
    try
    {
        obj6 = TestIntfPrx::checkedCast(base6);
        obj6->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;    

    cout << "testing identity indirect proxy... " << flush;
    obj->shutdown();
    manager->startServer();
    try
    {
        obj3 = TestIntfPrx::checkedCast(base3);
        obj3->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
        obj2->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
        obj2->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    try
    {
        obj3 = TestIntfPrx::checkedCast(base3);
        obj3->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();

    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
        obj2->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
        obj3 = TestIntfPrx::checkedCast(base3);
        obj3->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
        obj2->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();

    try
    {
        obj5 = TestIntfPrx::checkedCast(base5);
        obj5->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing proxy with unknown identity... " << flush;
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
    cout << "ok" << endl;

    cout << "testing proxy with unknown adapter... " << flush;
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
    cout << "ok" << endl;

    cout << "testing locator cache timeout... " << flush;

    int count = locator->getRequestCount();
    communicator->stringToProxy("test@TestAdapter")->ice_locatorCacheTimeout(0)->ice_ping(); // No locator cache.
    test(++count == locator->getRequestCount());
    communicator->stringToProxy("test@TestAdapter")->ice_locatorCacheTimeout(0)->ice_ping(); // No locator cache.
    test(++count == locator->getRequestCount());
    communicator->stringToProxy("test@TestAdapter")->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout.
    test(count == locator->getRequestCount());
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1200));
    communicator->stringToProxy("test@TestAdapter")->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout.
    test(++count == locator->getRequestCount());

    communicator->stringToProxy("test")->ice_locatorCacheTimeout(0)->ice_ping(); // No locator cache.
    count += 2;
    test(count == locator->getRequestCount());
    communicator->stringToProxy("test")->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout
    test(count == locator->getRequestCount());
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1200));
    communicator->stringToProxy("test")->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout
    count += 2;
    test(count == locator->getRequestCount());

    communicator->stringToProxy("test@TestAdapter")->ice_locatorCacheTimeout(-1)->ice_ping(); 
    test(count == locator->getRequestCount());
    communicator->stringToProxy("test")->ice_locatorCacheTimeout(-1)->ice_ping();
    test(count == locator->getRequestCount());
    communicator->stringToProxy("test@TestAdapter")->ice_ping(); 
    test(count == locator->getRequestCount());
    communicator->stringToProxy("test")->ice_ping();
    test(count == locator->getRequestCount());

    test(communicator->stringToProxy("test")->ice_locatorCacheTimeout(99)->ice_getLocatorCacheTimeout() == 99);

    cout << "ok" << endl;

    cout << "testing proxy from server... " << flush;
    HelloPrx hello = obj->getHello();
    test(hello->ice_getAdapterId() == "TestAdapter");
    hello->sayHello();
    hello = obj->getReplicatedHello();
    test(hello->ice_getAdapterId() == "ReplicatedAdapter");
    hello->sayHello();
    cout << "ok" << endl;

    cout << "testing proxy from server after shutdown... " << flush;
    obj->shutdown();
    manager->startServer();
    hello->sayHello();
    cout << "ok" << endl;

    cout << "testing object migration... " << flush;
    hello = HelloPrx::checkedCast(communicator->stringToProxy("hello"));
    obj->migrateHello();
    hello->sayHello();
    obj->migrateHello();
    hello->sayHello();
    obj->migrateHello();
    hello->sayHello();
    cout << "ok" << endl;

    cout << "shutdown server... " << flush;
    obj->shutdown();
    cout << "ok" << endl;

    cout << "testing whether server is gone... " << flush;
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
    cout << "ok" << endl;

    cout << "testing indirect proxies to collocated objects... " << flush;
    //
    // Set up test for calling a collocated object through an indirect, adapterless reference.
    //
    Ice::PropertiesPtr properties = communicator->getProperties();
    properties->setProperty("Ice.PrintAdapterReady", "0");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("Hello", "default");
    adapter->setLocator(locator);

    TestLocatorRegistryPrx registry = TestLocatorRegistryPrx::checkedCast(locator->getRegistry());
    test(registry);
    
    Ice::Identity id;
    id.name = IceUtil::generateUUID();
    registry->addObject(adapter->add(new HelloI, id));
    adapter->activate();
    
    try
    {
        HelloPrx helloPrx = HelloPrx::checkedCast(communicator->stringToProxy(communicator->identityToString(id)));
        Ice::ConnectionPtr connection = helloPrx->ice_getConnection();
        test(false);
    }
    catch(const Ice::CollocationOptimizationException&)
    {
    }
    adapter->deactivate();
    cout << "ok" << endl;

    cout << "shutdown server manager... " << flush;
    manager->shutdown();
    cout << "ok" << endl;
}
