// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Ice/Router.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <list>
#include <thread>

using namespace std;
using namespace Test;
using namespace Ice;

class HelloI : public virtual Hello
{
public:
    void sayHello(const Ice::Current&) override
    {
        // Do nothing, this is just a dummy servant.
    }
};

void
allTests(Test::TestHelper* helper, const string& ref)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    ServerManagerPrx manager(communicator, ref);
    auto locator = uncheckedCast<TestLocatorPrx>(communicator->getDefaultLocator().value());

    auto registry = uncheckedCast<TestLocatorRegistryPrx>(locator->getRegistry().value());

    cout << "testing stringToProxy... " << flush;
    ObjectPrx base(communicator, "test @ TestAdapter");
    ObjectPrx base2(communicator, "test @ TestAdapter");
    ObjectPrx base3(communicator, "test");
    ObjectPrx base4(communicator, "ServerManager");
    ObjectPrx base5(communicator, "test2");
    ObjectPrx base6(communicator, "test @ ReplicatedAdapter");
    cout << "ok" << endl;

    cout << "testing ice_locator and ice_getLocator... " << flush;
    test(Ice::proxyIdentityEqual(base->ice_getLocator(), communicator->getDefaultLocator()));
    auto anotherLocator = Ice::LocatorPrx(communicator, "anotherLocator");
    base = base->ice_locator(anotherLocator);
    test(Ice::proxyIdentityEqual(base->ice_getLocator(), anotherLocator));
    communicator->setDefaultLocator(nullopt);
    base = ObjectPrx(communicator, "test @ TestAdapter");
    test(!base->ice_getLocator());
    base = base->ice_locator(anotherLocator);
    test(Ice::proxyIdentityEqual(base->ice_getLocator(), anotherLocator));
    communicator->setDefaultLocator(locator);
    base = ObjectPrx(communicator, "test @ TestAdapter");
    test(Ice::proxyIdentityEqual(base->ice_getLocator(), communicator->getDefaultLocator()));

    //
    // We also test ice_router/ice_getRouter (perhaps we should add a
    // test/Ice/router test?)
    //
    test(!base->ice_getRouter());
    Ice::RouterPrx anotherRouter(communicator, "anotherRouter");
    base = base->ice_router(anotherRouter);
    test(Ice::proxyIdentityEqual(base->ice_getRouter(), anotherRouter));
    Ice::RouterPrx router(communicator, "dummyrouter");
    communicator->setDefaultRouter(router);
    base = ObjectPrx(communicator, "test @ TestAdapter");
    test(Ice::proxyIdentityEqual(base->ice_getRouter(), communicator->getDefaultRouter()));
    communicator->setDefaultRouter(nullopt);
    base = ObjectPrx(communicator, "test @ TestAdapter");
    test(!base->ice_getRouter());
    cout << "ok" << endl;

    cout << "starting server... " << flush;
    manager->startServer();
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    auto obj = Ice::checkedCast<TestIntfPrx>(base);
    test(obj);
    auto obj2 = Ice::checkedCast<TestIntfPrx>(base2);
    test(obj2);
    auto obj3 = Ice::checkedCast<TestIntfPrx>(base3);
    test(obj3);
    auto obj4 = Ice::checkedCast<ServerManagerPrx>(base4);
    test(obj4);
    auto obj5 = Ice::checkedCast<TestIntfPrx>(base5);
    test(obj5);
    auto obj6 = Ice::checkedCast<TestIntfPrx>(base6);
    test(obj6);
    cout << "ok" << endl;

    cout << "testing id@AdapterId indirect proxy... " << flush;
    obj->shutdown();
    manager->startServer();
    try
    {
        obj2->ice_ping();
    }
    catch (const Ice::LocalException& ex)
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
        obj6->ice_ping();
    }
    catch (const Ice::LocalException& ex)
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
        obj3->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    try
    {
        obj2->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
        obj2->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    try
    {
        obj3->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();

    try
    {
        obj2->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
        obj3->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();
    try
    {
        obj2->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    obj->shutdown();
    manager->startServer();

    try
    {
        obj5->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing proxy with unknown identity... " << flush;
    try
    {
        base = ObjectPrx(communicator, "unknown/unknown");
        base->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject() == "object");
        test(ex.id() == "unknown/unknown");
    }
    cout << "ok" << endl;

    cout << "testing proxy with unknown adapter... " << flush;
    try
    {
        base = ObjectPrx(communicator, "test @ TestAdapterUnknown");
        base->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject() == "object adapter");
        test(ex.id() == "TestAdapterUnknown");
    }
    cout << "ok" << endl;

    cout << "testing locator cache timeout... " << flush;

    int count = locator->getRequestCount();
    auto basencc = ObjectPrx(communicator, "test@TestAdapter")->ice_connectionCached(false);
    basencc->ice_locatorCacheTimeout(0)->ice_ping(); // No locator cache.
    test(++count == locator->getRequestCount());
    basencc->ice_locatorCacheTimeout(0)->ice_ping(); // No locator cache.
    test(++count == locator->getRequestCount());
    basencc->ice_locatorCacheTimeout(2)->ice_ping(); // 2s timeout.
    test(count == locator->getRequestCount());
    this_thread::sleep_for(chrono::milliseconds(1300));
    basencc->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout.
    test(++count == locator->getRequestCount());

    ObjectPrx(communicator, "test")->ice_locatorCacheTimeout(0)->ice_ping(); // No locator cache.
    count += 2;
    test(count == locator->getRequestCount());
    ObjectPrx(communicator, "test")->ice_locatorCacheTimeout(2)->ice_ping(); // 2s timeout
    test(count == locator->getRequestCount());
    this_thread::sleep_for(chrono::milliseconds(1300));
    ObjectPrx(communicator, "test")->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout
    count += 2;
    test(count == locator->getRequestCount());

    ObjectPrx(communicator, "test@TestAdapter")->ice_locatorCacheTimeout(-1)->ice_ping();
    test(count == locator->getRequestCount());
    ObjectPrx(communicator, "test")->ice_locatorCacheTimeout(-1)->ice_ping();
    test(count == locator->getRequestCount());
    ObjectPrx(communicator, "test@TestAdapter")->ice_ping();
    test(count == locator->getRequestCount());
    ObjectPrx(communicator, "test")->ice_ping();
    test(count == locator->getRequestCount());

    test(ObjectPrx(communicator, "test")->ice_locatorCacheTimeout(99)->ice_getLocatorCacheTimeout() == 99s);

    cout << "ok" << endl;

    cout << "testing proxy from server... " << flush;
    obj = TestIntfPrx(communicator, "test@TestAdapter");
    optional<HelloPrx> hello = obj->getHello();
    test(hello->ice_getAdapterId() == "TestAdapter");
    hello->sayHello();
    hello = obj->getReplicatedHello();
    test(hello->ice_getAdapterId() == "ReplicatedAdapter");
    hello->sayHello();
    cout << "ok" << endl;

    cout << "testing locator request queuing... " << flush;
    hello = obj->getReplicatedHello()->ice_locatorCacheTimeout(0)->ice_connectionCached(false);
    count = locator->getRequestCount();
    hello->ice_ping();
    test(++count == locator->getRequestCount());
    int i;

    list<future<void>> results;
    for (i = 0; i < 1000; i++)
    {
        auto result = make_shared<promise<void>>();
        hello->sayHelloAsync([result]() { result->set_value(); }, [result](exception_ptr) { test(false); });
        results.push_back(result->get_future());
    }
    for (auto& result : results)
    {
        result.get();
    }
    results.clear();
    test(locator->getRequestCount() > count && locator->getRequestCount() < count + 999);
    if (locator->getRequestCount() > count + 800)
    {
        cout << "queuing = " << locator->getRequestCount() - count;
    }
    count = locator->getRequestCount();
    hello = hello->ice_adapterId("unknown");
    for (i = 0; i < 1000; i++)
    {
        auto result = make_shared<promise<void>>();
        hello->sayHelloAsync(
            [result]() { test(false); },
            [result](exception_ptr ex)
            {
                try
                {
                    rethrow_exception(ex);
                }
                catch (const Ice::NotRegisteredException&)
                {
                    result->set_value();
                }
                catch (...)
                {
                    test(false);
                }
            });
        results.push_back(result->get_future());
    }
    for (auto& result : results)
    {
        result.get();
    }
    results.clear();
    // Take into account the retries.
    test(locator->getRequestCount() > count && locator->getRequestCount() < count + 1999);
    if (locator->getRequestCount() > count + 800)
    {
        cout << "queuing = " << locator->getRequestCount() - count;
    }
    cout << "ok" << endl;

    cout << "testing adapter locator cache... " << flush;
    try
    {
        ObjectPrx(communicator, "test@TestAdapter3")->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject() == "object adapter");
        test(ex.id() == "TestAdapter3");
    }
    registry->setAdapterDirectProxy("TestAdapter3", locator->findAdapterById("TestAdapter"));
    try
    {
        ObjectPrx(communicator, "test@TestAdapter3")->ice_ping();
        registry->setAdapterDirectProxy(
            "TestAdapter3",
            ObjectPrx(communicator, "dummy:" + helper->getTestEndpoint(99)));
        ObjectPrx(communicator, "test@TestAdapter3")->ice_ping();
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }

    try
    {
        ObjectPrx(communicator, "test@TestAdapter3")->ice_locatorCacheTimeout(0)->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    try
    {
        ObjectPrx(communicator, "test@TestAdapter3")->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    registry->setAdapterDirectProxy("TestAdapter3", locator->findAdapterById("TestAdapter"));
    try
    {
        ObjectPrx(communicator, "test@TestAdapter3")->ice_ping();
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing well-known object locator cache... " << flush;

    registry->addObject(ObjectPrx(communicator, "test3@TestUnknown"));
    try
    {
        ObjectPrx(communicator, "test3")->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject() == "object adapter");
        test(ex.id() == "TestUnknown");
    }
    registry->addObject(ObjectPrx(communicator, "test3@TestAdapter4")); // Update
    registry->setAdapterDirectProxy("TestAdapter4", ObjectPrx(communicator, "dummy:" + helper->getTestEndpoint(99)));
    try
    {
        ObjectPrx(communicator, "test3")->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    registry->setAdapterDirectProxy("TestAdapter4", locator->findAdapterById("TestAdapter"));
    try
    {
        ObjectPrx(communicator, "test3")->ice_ping();
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }

    registry->setAdapterDirectProxy("TestAdapter4", ObjectPrx(communicator, "dummy:" + helper->getTestEndpoint(99)));
    try
    {
        ObjectPrx(communicator, "test3")->ice_ping();
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }

    try
    {
        ObjectPrx(communicator, "test@TestAdapter4")->ice_locatorCacheTimeout(0)->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    try
    {
        ObjectPrx(communicator, "test@TestAdapter4")->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    try
    {
        ObjectPrx(communicator, "test3")->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    registry->addObject(ObjectPrx(communicator, "test3@TestAdapter"));
    try
    {
        ObjectPrx(communicator, "test3")->ice_ping();
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }

    registry->addObject(ObjectPrx(communicator, "test4"));
    try
    {
        ObjectPrx(communicator, "test4")->ice_ping();
        test(false);
    }
    catch (const Ice::NoEndpointException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing locator cache background updates... " << flush;
    {
        Ice::InitializationData initData;
        initData.properties = communicator->getProperties()->clone();
        initData.properties->setProperty("Ice.BackgroundLocatorCacheUpdates", "1");
        installTransport(initData);
        Ice::CommunicatorPtr ic = Ice::initialize(initData);

        registry->setAdapterDirectProxy("TestAdapter5", locator->findAdapterById("TestAdapter"));
        registry->addObject(ObjectPrx(communicator, "test3@TestAdapter"));

        count = locator->getRequestCount();
        ic->stringToProxy("test@TestAdapter5")->ice_locatorCacheTimeout(0)->ice_ping(); // No locator cache.
        ic->stringToProxy("test3")->ice_locatorCacheTimeout(0)->ice_ping();             // No locator cache.
        count += 3;
        test(count == locator->getRequestCount());
        registry->setAdapterDirectProxy("TestAdapter5", nullopt);
        registry->addObject(ObjectPrx(communicator, "test3:" + helper->getTestEndpoint(99)));
        ic->stringToProxy("test@TestAdapter5")->ice_locatorCacheTimeout(10)->ice_ping(); // 10s timeout.
        ic->stringToProxy("test3")->ice_locatorCacheTimeout(10)->ice_ping();             // 10s timeout.
        test(count == locator->getRequestCount());
        this_thread::sleep_for(chrono::milliseconds(1200));

        // The following request should trigger the background updates but still use the cached endpoints
        // and therefore succeed.
        ic->stringToProxy("test@TestAdapter5")->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout.
        ic->stringToProxy("test3")->ice_locatorCacheTimeout(1)->ice_ping();             // 1s timeout.

        try
        {
            while (true)
            {
                ic->stringToProxy("test@TestAdapter5")->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout.
                this_thread::sleep_for(chrono::milliseconds(10));
            }
        }
        catch (const Ice::LocalException&)
        {
            // Expected to fail once they endpoints have been updated in the background.
        }
        try
        {
            while (true)
            {
                ic->stringToProxy("test3")->ice_locatorCacheTimeout(1)->ice_ping(); // 1s timeout.
                this_thread::sleep_for(chrono::milliseconds(10));
            }
        }
        catch (const Ice::LocalException&)
        {
            // Expected to fail once they endpoints have been updated in the background.
        }
        ic->destroy();
    }
    cout << "ok" << endl;

    cout << "testing proxy from server after shutdown... " << flush;
    hello = obj->getReplicatedHello();
    obj->shutdown();
    manager->startServer();
    hello->sayHello();
    cout << "ok" << endl;

    cout << "testing object migration... " << flush;
    hello = HelloPrx(communicator, "hello");
    obj->migrateHello();
    hello->ice_getConnection()->close().get();
    hello->sayHello();
    obj->migrateHello();
    hello->sayHello();
    obj->migrateHello();
    hello->sayHello();
    cout << "ok" << endl;

    cout << "testing locator encoding resolution... " << flush;

    hello = HelloPrx(communicator, "hello");
    count = locator->getRequestCount();
    ObjectPrx(communicator, "test@TestAdapter")->ice_encodingVersion(Ice::Encoding_1_1)->ice_ping();
    test(count == locator->getRequestCount());
    ObjectPrx(communicator, "test@TestAdapter10")->ice_encodingVersion(Ice::Encoding_1_0)->ice_ping();
    test(++count == locator->getRequestCount());
    ObjectPrx(communicator, "test -e 1.0@TestAdapter10-2")->ice_ping();
    test(++count == locator->getRequestCount());

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
    catch (const Ice::LocalException&)
    {
    }
    try
    {
        obj3->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    try
    {
        obj5->ice_ping();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    cout << "ok" << endl;

    {
        cout << "testing indirect proxies to collocated objects... " << flush;

        communicator->getProperties()->setProperty("Hello.AdapterId", Ice::generateUUID());
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("Hello", "default");

        Ice::Identity id;
        id.name = Ice::generateUUID();
        adapter->add(std::make_shared<HelloI>(), id);

        // Ensure that calls on the well-known proxy is collocated.
        HelloPrx helloPrx(communicator, communicator->identityToString(id));
        test(!helloPrx->ice_getConnection());

        // Ensure that calls on the indirect proxy (with adapter ID) is collocated
        helloPrx = adapter->createIndirectProxy<HelloPrx>(id);
        test(!helloPrx->ice_getConnection());

        // Ensure that calls on the direct proxy is collocated
        helloPrx = adapter->createDirectProxy<HelloPrx>(id);
        test(!helloPrx->ice_getConnection());

        cout << "ok" << endl;
    }

    cout << "testing indirect object adapter without published endpoints... " << flush;
    int32_t setRequestCount = registry.getSetRequestCount();
    communicator->getProperties()->setProperty("CollocAdapter.AdapterId", "CollocId");
    ObjectAdapterPtr collocAdapter = communicator->createObjectAdapter("CollocAdapter");
    collocAdapter->activate();                               // not necessary, but allowed
    test(setRequestCount == registry->getSetRequestCount()); // no set call on registry
    collocAdapter->deactivate();
    test(setRequestCount == registry->getSetRequestCount()); // no set call on registry
    cout << "ok" << endl;

    cout << "shutdown server manager... " << flush;
    manager->shutdown();
    cout << "ok" << endl;
}
