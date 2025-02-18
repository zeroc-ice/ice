// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

function proxyIdentityCompare(p1: Ice.ObjectPrx, p2: Ice.ObjectPrx): boolean {
    return p1.ice_getIdentity().equals(p2.ice_getIdentity());
}

export class Client extends TestHelper {
    async allTests() {
        const communicator = this.communicator();
        const out = this.getWriter();
        const manager = new Test.ServerManagerPrx(communicator, `ServerManager:${this.getTestEndpoint()}`);

        const locator = new Test.TestLocatorPrx(communicator.getDefaultLocator());

        const registry = new Test.TestLocatorRegistryPrx((await locator.getRegistry())!);

        out.write("testing stringToProxy... ");
        let base = communicator.stringToProxy("test @ TestAdapter");
        const base2 = communicator.stringToProxy("test @ TestAdapter");
        const base3 = communicator.stringToProxy("test");
        const base4 = communicator.stringToProxy("ServerManager");
        const base5 = communicator.stringToProxy("test2");
        const base6 = communicator.stringToProxy("test @ ReplicatedAdapter");
        out.writeLine("ok");

        out.write("testing ice_locator and ice_getLocator... ");
        test(proxyIdentityCompare(base.ice_getLocator(), communicator.getDefaultLocator()));
        const anotherLocator = Ice.LocatorPrx.uncheckedCast(communicator.stringToProxy("anotherLocator"));
        base = base.ice_locator(anotherLocator);
        test(proxyIdentityCompare(base.ice_getLocator(), anotherLocator));
        communicator.setDefaultLocator(null);
        base = communicator.stringToProxy("test @ TestAdapter");
        test(base.ice_getLocator() === null);
        base = base.ice_locator(anotherLocator);
        test(proxyIdentityCompare(base.ice_getLocator(), anotherLocator));
        communicator.setDefaultLocator(locator);
        base = communicator.stringToProxy("test @ TestAdapter");
        test(proxyIdentityCompare(base.ice_getLocator(), communicator.getDefaultLocator()));

        //
        // We also test ice_router/ice_getRouter (perhaps we should add a
        // test/Ice/router test?)
        //
        test(base.ice_getRouter() === null);
        const anotherRouter = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("anotherRouter"));
        base = base.ice_router(anotherRouter);
        test(proxyIdentityCompare(base.ice_getRouter(), anotherRouter));
        const router = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("dummyrouter"));
        communicator.setDefaultRouter(router);
        base = communicator.stringToProxy("test @ TestAdapter");
        test(proxyIdentityCompare(base.ice_getRouter(), communicator.getDefaultRouter()));
        communicator.setDefaultRouter(null);
        base = communicator.stringToProxy("test @ TestAdapter");
        test(base.ice_getRouter() === null);
        out.writeLine("ok");

        out.write("starting server... ");
        await manager.startServer();
        out.writeLine("ok");

        out.write("testing checked cast... ");
        let obj = await Test.TestIntfPrx.checkedCast(base);
        test(obj !== null);

        const obj2 = await Test.TestIntfPrx.checkedCast(base2);
        test(obj2 !== null);

        const obj3 = await Test.TestIntfPrx.checkedCast(base3);
        test(obj3 !== null);

        const obj4 = await Test.ServerManagerPrx.checkedCast(base4);
        test(obj4 !== null);

        const obj5 = await Test.TestIntfPrx.checkedCast(base5);
        test(obj5 !== null);

        const obj6 = await Test.TestIntfPrx.checkedCast(base6);
        test(obj6 !== null);
        out.writeLine("ok");

        out.write("testing id@AdapterId indirect proxy... ");
        await obj!.shutdown();
        await manager.startServer();
        await obj2!.ice_ping();
        out.writeLine("ok");

        out.write("testing id@ReplicaGroupId indirect proxy... ");
        await obj!.shutdown();
        await manager.startServer();
        await obj6!.ice_ping();
        out.writeLine("ok");

        out.write("testing identity indirect proxy... ");
        await obj!.shutdown();
        await manager.startServer();
        await obj3!.ice_ping();
        await obj2!.ice_ping();
        await obj!.shutdown();
        await manager.startServer();
        await obj2!.ice_ping();
        await obj3!.ice_ping();
        await obj!.shutdown();
        await manager.startServer();
        await obj2!.ice_ping();
        await obj!.shutdown();
        await manager.startServer();
        await obj3!.ice_ping();
        await obj!.shutdown();
        await manager.startServer();
        await obj2!.ice_ping();
        await obj!.shutdown();
        await manager.startServer();
        await obj5!.ice_ping();
        out.writeLine("ok");

        out.write("testing proxy with unknown identity... ");
        base = communicator.stringToProxy("unknown/unknown");
        try {
            await base.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.NotRegisteredException, ex);
            test(ex.kindOfObject == "object");
            test(ex.id == "unknown/unknown");
        }
        out.writeLine("ok");

        out.write("testing proxy with unknown adapter... ");
        base = communicator.stringToProxy("test @ TestAdapterUnknown");
        try {
            await base.ice_ping();
        } catch (ex) {
            test(ex instanceof Ice.NotRegisteredException, ex);
            test(ex.kindOfObject == "object adapter");
            test(ex.id == "TestAdapterUnknown");
        }
        out.writeLine("ok");

        out.write("testing locator cache timeout... ");
        let count = await locator.getRequestCount();
        const basencc = communicator.stringToProxy("test@TestAdapter").ice_connectionCached(false);
        await basencc.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == (await locator.getRequestCount()));
        await basencc.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == (await locator.getRequestCount()));
        await basencc.ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(count == (await locator.getRequestCount()));
        await Ice.Promise.delay(1200); // 1200ms
        await basencc.ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(++count == (await locator.getRequestCount()));

        await communicator.stringToProxy("test").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        count += 2;
        test(count == (await locator.getRequestCount()));
        await communicator.stringToProxy("test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        test(count == (await locator.getRequestCount()));
        await Ice.Promise.delay(1200); // 1200ms
        await communicator.stringToProxy("test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        count += 2;
        test(count == (await locator.getRequestCount()));

        await communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(-1).ice_ping();
        test(count == (await locator.getRequestCount()));
        await communicator.stringToProxy("test").ice_locatorCacheTimeout(-1).ice_ping();
        test(count == (await locator.getRequestCount()));
        await communicator.stringToProxy("test@TestAdapter").ice_ping();
        test(count == (await locator.getRequestCount()));
        await communicator.stringToProxy("test").ice_ping();
        test(count == (await locator.getRequestCount()));

        test(communicator.stringToProxy("test").ice_locatorCacheTimeout(99).ice_getLocatorCacheTimeout() === 99);
        out.writeLine("ok");

        out.write("testing proxy from server... ");
        obj = await Test.TestIntfPrx.checkedCast(communicator.stringToProxy("test@TestAdapter"));
        let hello = await obj!.getHello();
        test(hello!.ice_getAdapterId() == "TestAdapter");
        await hello!.sayHello();
        hello = await obj!.getReplicatedHello();
        test(hello!.ice_getAdapterId() == "ReplicatedAdapter");
        await hello!.sayHello();
        out.writeLine("ok");

        out.write("testing locator request queuing... ");
        hello = await obj!.getReplicatedHello();
        hello = hello!.ice_locatorCacheTimeout(0).ice_connectionCached(false);
        count = await locator.getRequestCount();
        await hello.ice_ping();
        test(++count == (await locator.getRequestCount()));

        let results: Promise<void>[] = [];
        for (let i = 0; i < 1000; i++) {
            results.push(
                hello.sayHello().catch((ex) => {
                    test(false);
                }),
            );
        }
        await Promise.all(results);

        results = [];
        test((await locator.getRequestCount()) > count && (await locator.getRequestCount()) < count + 999);

        if ((await locator.getRequestCount()) > count + 800) {
            out.write("queuing = " + ((await locator.getRequestCount()) - count));
        }
        count = await locator.getRequestCount();
        hello = hello.ice_adapterId("unknown");
        for (let i = 0; i < 1000; i++) {
            results.push(
                hello.sayHello().then(
                    () => {
                        test(false);
                    },
                    (ex) => {
                        test(ex instanceof Ice.NotRegisteredException, ex);
                    },
                ),
            );
        }
        await Promise.all(results);
        results = [];
        // XXX:
        // Take into account the retries.
        test((await locator.getRequestCount()) > count && (await locator.getRequestCount()) < count + 1999);
        if ((await locator.getRequestCount()) > count + 800) {
            out.write("queuing = " + ((await locator.getRequestCount()) - count));
        }
        out.writeLine("ok");

        out.write("testing adapter locator cache... ");
        try {
            await communicator.stringToProxy("test@TestAdapter3").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.NotRegisteredException, ex);
            test(ex.kindOfObject == "object adapter");
            test(ex.id == "TestAdapter3");
        }
        await registry.setAdapterDirectProxy("TestAdapter3", await locator.findAdapterById("TestAdapter"));
        try {
            await communicator.stringToProxy("test@TestAdapter3").ice_ping();
            await registry.setAdapterDirectProxy("TestAdapter3", communicator.stringToProxy("dummy:default"));
            await communicator.stringToProxy("test@TestAdapter3").ice_ping();
        } catch (ex) {
            test(false, ex);
        }

        try {
            await communicator.stringToProxy("test@TestAdapter3").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex);
        }
        try {
            await communicator.stringToProxy("test@TestAdapter3").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex);
        }
        await registry.setAdapterDirectProxy("TestAdapter3", await locator.findAdapterById("TestAdapter"));
        try {
            await communicator.stringToProxy("test@TestAdapter3").ice_ping();
        } catch (ex) {
            test(false, ex);
        }

        out.writeLine("ok");

        out.write("testing well-known object locator cache... ");
        await registry.addObject(communicator.stringToProxy("test3@TestUnknown"));
        try {
            await communicator.stringToProxy("test3").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.NotRegisteredException, ex);
            test(ex.kindOfObject == "object adapter");
            test(ex.id == "TestUnknown");
        }
        await registry.addObject(communicator.stringToProxy("test3@TestAdapter4")); // Update
        await registry.setAdapterDirectProxy("TestAdapter4", communicator.stringToProxy("dummy:default"));
        try {
            await communicator.stringToProxy("test3").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex);
        }

        await registry.setAdapterDirectProxy("TestAdapter4", await locator.findAdapterById("TestAdapter"));
        try {
            await communicator.stringToProxy("test3").ice_ping();
        } catch (ex) {
            test(false, ex);
        }

        await registry.setAdapterDirectProxy("TestAdapter4", communicator.stringToProxy("dummy:default"));
        try {
            await communicator.stringToProxy("test3").ice_ping();
        } catch (ex) {
            test(false, ex);
        }

        try {
            await communicator.stringToProxy("test@TestAdapter4").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex);
        }

        try {
            await communicator.stringToProxy("test@TestAdapter4").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException);
        }

        try {
            await communicator.stringToProxy("test3").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex);
        }

        await registry.addObject(communicator.stringToProxy("test3@TestAdapter"));
        try {
            await communicator.stringToProxy("test3").ice_ping();
        } catch (ex) {
            test(false, ex);
        }

        await registry.addObject(communicator.stringToProxy("test4"));
        try {
            await communicator.stringToProxy("test4").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.NoEndpointException, ex);
        }
        out.writeLine("ok");

        out.write("testing locator cache background updates... ");
        {
            const initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().clone();
            initData.properties.setProperty("Ice.BackgroundLocatorCacheUpdates", "1");
            const ic = Ice.initialize(initData);

            await registry.setAdapterDirectProxy("TestAdapter5", await locator.findAdapterById("TestAdapter"));
            await registry.addObject(communicator.stringToProxy("test3@TestAdapter"));

            count = await locator.getRequestCount();
            await ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
            await ic.stringToProxy("test3").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
            count += 3;
            test(count == (await locator.getRequestCount()));
            await registry.setAdapterDirectProxy("TestAdapter5", null);
            await registry.addObject(communicator.stringToProxy("test3:default"));
            await ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
            await ic.stringToProxy("test3").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
            test(count == (await locator.getRequestCount()));
            await Ice.Promise.delay(1200);

            // The following request should trigger the background
            // updates but still use the cached endpoints and
            // therefore succeed.
            await ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
            await ic.stringToProxy("test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.

            try {
                while (true) {
                    await ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                    await Ice.Promise.delay(10);
                }
            } catch (ex) {
                // Expected to fail once they endpoints have been updated in the background.
                test(ex instanceof Ice.LocalException, ex);
            }
            try {
                while (true) {
                    await ic.stringToProxy("test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                    await Ice.Promise.delay(10);
                }
            } catch (ex) {
                // Expected to fail once they endpoints have been updated in the background.
                test(ex instanceof Ice.LocalException, ex);
            }
            await ic.destroy();
        }
        out.writeLine("ok");

        out.write("testing proxy from server after shutdown... ");
        hello = await obj!.getReplicatedHello();
        await obj!.shutdown();
        await manager.startServer();
        await hello!.sayHello();
        out.writeLine("ok");

        out.write("testing object migration... ");
        hello = await Test.HelloPrx.checkedCast(communicator.stringToProxy("hello"));
        await obj!.migrateHello();
        const conn = await hello!.ice_getConnection();
        await conn.close();
        await hello!.sayHello();
        await obj!.migrateHello();
        await hello!.sayHello();
        await obj!.migrateHello();
        await hello!.sayHello();
        out.writeLine("ok");

        out.write("testing locator encoding resolution... ");
        hello = await Test.HelloPrx.checkedCast(communicator.stringToProxy("hello"));
        count = await locator.getRequestCount();
        await communicator.stringToProxy("test@TestAdapter").ice_encodingVersion(Ice.Encoding_1_1).ice_ping();
        test(count == (await locator.getRequestCount()));
        await communicator.stringToProxy("test@TestAdapter10").ice_encodingVersion(Ice.Encoding_1_0).ice_ping();
        test(++count == (await locator.getRequestCount()));
        await communicator.stringToProxy("test -e 1.0@TestAdapter10-2").ice_ping();
        test(++count == (await locator.getRequestCount()));
        out.writeLine("ok");

        out.write("shutdown server manager... ");
        await manager.shutdown();
        out.writeLine("ok");
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Default.Locator", "locator:" + this.getTestEndpoint(properties));
            [communicator] = this.initialize(properties);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
