// Copyright (c) ZeroC, Inc.

package test.Ice.location;

import com.zeroc.Ice.AdapterAlreadyActiveException;
import com.zeroc.Ice.AdapterNotFoundException;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.LocatorPrx;
import com.zeroc.Ice.NoEndpointException;
import com.zeroc.Ice.NotRegisteredException;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.RouterPrx;
import com.zeroc.Ice.Util;

import test.Ice.location.Test.HelloPrx;
import test.Ice.location.Test.ServerManagerPrx;
import test.Ice.location.Test.TestIntfPrx;
import test.Ice.location.Test.TestLocatorPrx;
import test.Ice.location.Test.TestLocatorRegistryPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.time.Duration;
import java.util.LinkedList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(TestHelper helper)
        throws AdapterAlreadyActiveException,
        AdapterNotFoundException,
        InterruptedException {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        ServerManagerPrx manager =
            ServerManagerPrx.checkedCast(
                communicator.stringToProxy("ServerManager:" + helper.getTestEndpoint(0)));
        test(manager != null);

        TestLocatorPrx locator = TestLocatorPrx.uncheckedCast(communicator.getDefaultLocator());
        test(locator != null);

        TestLocatorRegistryPrx registry = TestLocatorRegistryPrx.checkedCast(locator.getRegistry());
        test(registry != null);

        out.print("testing stringToProxy... ");
        out.flush();
        ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
        final ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
        final ObjectPrx base3 = communicator.stringToProxy("test");
        final ObjectPrx base4 = communicator.stringToProxy("ServerManager");
        final ObjectPrx base5 = communicator.stringToProxy("test2");
        final ObjectPrx base6 = communicator.stringToProxy("test @ ReplicatedAdapter");
        out.println("ok");

        out.print("testing ice_locator and ice_getLocator... ");
        test(
            Util.proxyIdentityCompare(base.ice_getLocator(), communicator.getDefaultLocator())
                == 0);
        var anotherLocator = LocatorPrx.createProxy(communicator, "anotherLocator");
        base = base.ice_locator(anotherLocator);
        test(Util.proxyIdentityCompare(base.ice_getLocator(), anotherLocator) == 0);
        communicator.setDefaultLocator(null);
        base = communicator.stringToProxy("test @ TestAdapter");
        test(base.ice_getLocator() == null);
        base = base.ice_locator(anotherLocator);
        test(Util.proxyIdentityCompare(base.ice_getLocator(), anotherLocator) == 0);
        communicator.setDefaultLocator(locator);
        base = communicator.stringToProxy("test @ TestAdapter");
        test(
            Util.proxyIdentityCompare(base.ice_getLocator(), communicator.getDefaultLocator())
                == 0);

        //
        // We also test ice_router/ice_getRouter (perhaps we should add a test/Ice/router test?)
        //
        test(base.ice_getRouter() == null);
        var anotherRouter = RouterPrx.createProxy(communicator, "anotherRouter");
        base = base.ice_router(anotherRouter);
        test(Util.proxyIdentityCompare(base.ice_getRouter(), anotherRouter) == 0);
        var router = RouterPrx.createProxy(communicator, "dummyrouter");
        communicator.setDefaultRouter(router);
        base = communicator.stringToProxy("test @ TestAdapter");
        test(Util.proxyIdentityCompare(base.ice_getRouter(), communicator.getDefaultRouter()) == 0);
        communicator.setDefaultRouter(null);
        base = communicator.stringToProxy("test @ TestAdapter");
        test(base.ice_getRouter() == null);
        out.println("ok");

        out.print("starting server... ");
        out.flush();
        manager.startServer();
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrx.checkedCast(base);
        test(obj != null);
        TestIntfPrx obj2 = TestIntfPrx.checkedCast(base2);
        test(obj2 != null);
        TestIntfPrx obj3 = TestIntfPrx.checkedCast(base3);
        test(obj3 != null);
        ServerManagerPrx obj4 = ServerManagerPrx.checkedCast(base4);
        test(obj4 != null);
        TestIntfPrx obj5 = TestIntfPrx.checkedCast(base5);
        test(obj5 != null);
        TestIntfPrx obj6 = TestIntfPrx.checkedCast(base6);
        test(obj6 != null);
        out.println("ok");

        out.print("testing id@AdapterId indirect proxy... ");
        out.flush();
        obj.shutdown();
        manager.startServer();
        try {
            obj2.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        out.println("ok");

        out.print("testing id@ReplicaGroupId indirect proxy... ");
        out.flush();
        obj.shutdown();
        manager.startServer();
        try {
            obj6.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        out.println("ok");

        out.print("testing identity indirect proxy... ");
        out.flush();
        obj.shutdown();
        manager.startServer();
        try {
            obj3.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        try {
            obj2.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        obj.shutdown();
        manager.startServer();
        try {
            obj2.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        try {
            obj3.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        obj.shutdown();
        manager.startServer();

        try {
            obj2.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        obj.shutdown();
        manager.startServer();
        try {
            obj3.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        obj.shutdown();
        manager.startServer();
        try {
            obj2.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        obj.shutdown();
        manager.startServer();

        try {
            obj5.ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        out.println("ok");

        out.print("testing proxy with unknown identity... ");
        out.flush();
        try {
            base = communicator.stringToProxy("unknown/unknown");
            base.ice_ping();
            test(false);
        } catch (NotRegisteredException ex) {
            test("object".equals(ex.kindOfObject));
            test("unknown/unknown".equals(ex.id));
        }
        out.println("ok");

        out.print("testing proxy with unknown adapter... ");
        out.flush();
        try {
            base = communicator.stringToProxy("test @ TestAdapterUnknown");
            base.ice_ping();
            test(false);
        } catch (NotRegisteredException ex) {
            test("object adapter".equals(ex.kindOfObject));
            test("TestAdapterUnknown".equals(ex.id));
        }
        out.println("ok");

        out.print("testing locator cache timeout... ");
        out.flush();
        ObjectPrx basencc =
            communicator.stringToProxy("test@TestAdapter").ice_connectionCached(false);
        int count = locator.getRequestCount();
        basencc.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == locator.getRequestCount());
        basencc.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == locator.getRequestCount());
        basencc.ice_locatorCacheTimeout(2).ice_ping(); // 2s timeout.
        test(count == locator.getRequestCount());
        Thread.sleep(1300);
        basencc.ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(++count == locator.getRequestCount());

        communicator
            .stringToProxy("test")
            .ice_locatorCacheTimeout(0)
            .ice_ping(); // No locator cache.
        count += 2;
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test").ice_locatorCacheTimeout(2).ice_ping(); // 2s timeout
        test(count == locator.getRequestCount());
        Thread.sleep(1300);
        communicator.stringToProxy("test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        count += 2;
        test(count == locator.getRequestCount());

        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(-1).ice_ping();
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test").ice_locatorCacheTimeout(-1).ice_ping();
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter").ice_ping();
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test").ice_ping();
        test(count == locator.getRequestCount());

        test(
            communicator
                .stringToProxy("test")
                .ice_locatorCacheTimeout(Duration.ofSeconds(99))
                .ice_getLocatorCacheTimeout()
                .equals(Duration.ofSeconds(99)));

        out.println("ok");

        out.print("testing proxy from server... ");
        out.flush();
        obj = TestIntfPrx.checkedCast(communicator.stringToProxy("test@TestAdapter"));
        HelloPrx hello = obj.getHello();
        test("TestAdapter".equals(hello.ice_getAdapterId()));
        hello = obj.getReplicatedHello();
        test("ReplicatedAdapter".equals(hello.ice_getAdapterId()));
        hello.sayHello();
        out.println("ok");

        out.print("testing proxy from server after shutdown... ");
        out.flush();
        obj.shutdown();
        manager.startServer();
        hello.sayHello();
        out.println("ok");

        out.print("testing locator request queuing... ");
        out.flush();
        hello = obj.getReplicatedHello().ice_locatorCacheTimeout(0).ice_connectionCached(false);
        count = locator.getRequestCount();
        hello.ice_ping();
        test(++count == locator.getRequestCount());
        List<CompletableFuture<Void>> results = new LinkedList<>();
        for (int i = 0; i < 1000; i++) {
            CompletableFuture<Void> f = hello.sayHelloAsync();
            f.whenComplete(
                (result, ex) -> {
                    if (ex != null) {
                        ex.printStackTrace();
                        test(false);
                    }
                });
            results.add(f);
        }
        while (!results.isEmpty()) {
            CompletableFuture<Void> r = results.remove(0);
            r.join();
        }
        test(locator.getRequestCount() > count && locator.getRequestCount() < count + 999);
        if (locator.getRequestCount() > count + 800) {
            out.print("queuing = " + (locator.getRequestCount() - count));
        }
        count = locator.getRequestCount();
        hello = hello.ice_adapterId("unknown");
        for (int i = 0; i < 1000; i++) {
            CompletableFuture<Void> f = hello.sayHelloAsync();
            f.whenComplete(
                (result, ex) -> {
                    test(ex != null && ex instanceof NotRegisteredException);
                });
            results.add(f);
        }
        while (!results.isEmpty()) {
            CompletableFuture<Void> r = results.remove(0);
            try {
                r.join();
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof NotRegisteredException);
            }
        }
        // Take into account the retries.
        test(locator.getRequestCount() > count && locator.getRequestCount() < count + 1999);
        if (locator.getRequestCount() > count + 800) {
            out.print("queuing = " + (locator.getRequestCount() - count));
        }
        out.println("ok");

        out.print("testing adapter locator cache... ");
        out.flush();
        try {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            test(false);
        } catch (NotRegisteredException ex) {
            test(ex.kindOfObject == "object adapter");
            test("TestAdapter3".equals(ex.id));
        }
        registry.setAdapterDirectProxy("TestAdapter3", locator.findAdapterById("TestAdapter"));
        try {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            registry.setAdapterDirectProxy(
                "TestAdapter3",
                communicator.stringToProxy("dummy:" + helper.getTestEndpoint(99)));
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            communicator.stringToProxy("test@TestAdapter3").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        } catch (LocalException ex) {}
        try {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            test(false);
        } catch (LocalException ex) {}
        registry.setAdapterDirectProxy("TestAdapter3", locator.findAdapterById("TestAdapter"));
        try {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        out.println("ok");

        out.print("testing well-known object locator cache... ");
        out.flush();

        registry.addObject(communicator.stringToProxy("test3@TestUnknown"));
        try {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        } catch (NotRegisteredException ex) {
            test(ex.kindOfObject == "object adapter");
            test("TestUnknown".equals(ex.id));
        }
        registry.addObject(communicator.stringToProxy("test3@TestAdapter4")); // Update
        registry.setAdapterDirectProxy(
            "TestAdapter4", communicator.stringToProxy("dummy:" + helper.getTestEndpoint(99)));

        try {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        } catch (LocalException ex) {}
        registry.setAdapterDirectProxy("TestAdapter4", locator.findAdapterById("TestAdapter"));
        try {
            communicator.stringToProxy("test3").ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        registry.setAdapterDirectProxy(
            "TestAdapter4", communicator.stringToProxy("dummy:" + helper.getTestEndpoint(99)));
        try {
            communicator.stringToProxy("test3").ice_ping();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            communicator.stringToProxy("test@TestAdapter4").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        } catch (LocalException ex) {}
        try {
            communicator.stringToProxy("test@TestAdapter4").ice_ping();
            test(false);
        } catch (LocalException ex) {}
        try {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        } catch (LocalException ex) {}
        registry.addObject(communicator.stringToProxy("test3@TestAdapter"));
        try {
            communicator.stringToProxy("test3").ice_ping();
        } catch (LocalException ex) {
            test(false);
        }

        registry.addObject(communicator.stringToProxy("test4"));
        try {
            communicator.stringToProxy("test4").ice_ping();
            test(false);
        } catch (NoEndpointException ex) {}
        out.println("ok");

        out.print("testing locator cache background updates... ");
        out.flush();
        {
            Properties properties = communicator.getProperties()._clone();
            properties.setProperty("Ice.BackgroundLocatorCacheUpdates", "1");
            try (Communicator ic = helper.initialize(properties)) {
                registry.setAdapterDirectProxy(
                    "TestAdapter5", locator.findAdapterById("TestAdapter"));
                registry.addObject(communicator.stringToProxy("test3@TestAdapter"));

                count = locator.getRequestCount();
                ic.stringToProxy("test@TestAdapter5")
                    .ice_locatorCacheTimeout(0)
                    .ice_ping(); // No locator cache.
                ic.stringToProxy("test3")
                    .ice_locatorCacheTimeout(0)
                    .ice_ping(); // No locator cache.
                count += 3;
                test(count == locator.getRequestCount());
                registry.setAdapterDirectProxy("TestAdapter5", null);
                registry.addObject(
                    communicator.stringToProxy("test3:" + helper.getTestEndpoint(99)));
                ic.stringToProxy("test@TestAdapter5")
                    .ice_locatorCacheTimeout(10)
                    .ice_ping(); // 10s timeout.
                ic.stringToProxy("test3").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
                test(count == locator.getRequestCount());
                Thread.sleep(1200);

                // The following request should trigger the background updates but still use the
                // cached endpoints and therefore succeed.
                ic.stringToProxy("test@TestAdapter5")
                    .ice_locatorCacheTimeout(1)
                    .ice_ping(); // 1s timeout.
                ic.stringToProxy("test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.

                try {
                    while (true) {
                        ic.stringToProxy("test@TestAdapter5")
                            .ice_locatorCacheTimeout(1)
                            .ice_ping(); // 1s timeout.
                        Thread.sleep(10);
                    }
                } catch (LocalException ex) {
                    // Expected to fail once they endpoints have been updated in the background.
                }

                try {
                    while (true) {
                        ic.stringToProxy("test3")
                            .ice_locatorCacheTimeout(1)
                            .ice_ping(); // 1s timeout.
                        Thread.sleep(10);
                    }
                } catch (LocalException ex) {
                    // Expected to fail once they endpoints have been updated in the background.
                }
            }
        }
        out.println("ok");

        out.print("testing proxy from server after shutdown... ");
        out.flush();
        hello = obj.getReplicatedHello();
        obj.shutdown();
        manager.startServer();
        hello.sayHello();
        out.println("ok");

        out.print("testing object migration...");
        out.flush();
        hello = HelloPrx.checkedCast(communicator.stringToProxy("hello"));
        obj.migrateHello();
        hello.ice_getConnection().close();
        hello.sayHello();
        obj.migrateHello();
        hello.sayHello();
        obj.migrateHello();
        hello.sayHello();
        out.println("ok");

        out.print("testing locator encoding resolution... ");
        out.flush();
        hello = HelloPrx.checkedCast(communicator.stringToProxy("hello"));
        count = locator.getRequestCount();
        communicator
            .stringToProxy("test@TestAdapter")
            .ice_encodingVersion(Util.Encoding_1_1)
            .ice_ping();
        test(count == locator.getRequestCount());
        communicator
            .stringToProxy("test@TestAdapter10")
            .ice_encodingVersion(Util.Encoding_1_0)
            .ice_ping();
        test(++count == locator.getRequestCount());
        communicator.stringToProxy("test -e 1.0@TestAdapter10-2").ice_ping();
        test(++count == locator.getRequestCount());
        out.println("ok");

        out.print("shutdown server... ");
        out.flush();
        obj.shutdown();
        out.println("ok");

        out.print("testing whether server is gone... ");
        out.flush();
        try {
            obj2.ice_ping();
            test(false);
        } catch (LocalException ex) {}

        try {
            obj3.ice_ping();
            test(false);
        } catch (LocalException ex) {}

        try {
            obj5.ice_ping();
            test(false);
        } catch (LocalException ex) {}
        out.println("ok");

        out.print("testing indirect proxies to collocated objects... ");

        communicator
            .getProperties()
            .setProperty("Hello.AdapterId", UUID.randomUUID().toString());
        ObjectAdapter adapter =
            communicator.createObjectAdapterWithEndpoints("Hello", "default");

        Identity id = new Identity();
        id.name = UUID.randomUUID().toString();
        adapter.add(new HelloI(), id);

        // Ensure that calls on the well-known proxy is collocated.
        HelloPrx helloPrx =
            HelloPrx.checkedCast(
                communicator.stringToProxy(
                    "\"" + communicator.identityToString(id) + "\""));
        test(helloPrx.ice_getConnection() == null);

        // Ensure that calls on the indirect proxy (with adapter ID) is collocated
        helloPrx = HelloPrx.checkedCast(adapter.createIndirectProxy(id));
        test(helloPrx.ice_getConnection() == null);

        // Ensure that calls on the direct proxy is collocated
        helloPrx = HelloPrx.checkedCast(adapter.createDirectProxy(id));
        test(helloPrx.ice_getConnection() == null);

        out.println("ok");

        out.print("shutdown server manager... ");
        out.flush();
        manager.shutdown();
        out.println("ok");
    }

    private AllTests() {}
}
