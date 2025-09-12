// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper, test } from "../../Common/TestHelper.js";

function proxyIdentityCompare(p1: Ice.ObjectPrx, p2: Ice.ObjectPrx): boolean {
    return p1.ice_getIdentity().equals(p2.ice_getIdentity());
}

export class Client extends TestHelper {
    async allTests() {
        const communicator = this.communicator();
        const out = this.getWriter();
        const manager = new Test.ServerManagerPrx(communicator, `ServerManager:${this.getTestEndpoint()}`);

        const locator = Test.TestLocatorPrx.uncheckedCast(communicator.getDefaultLocator()!);

        const registry = Test.TestLocatorRegistryPrx.uncheckedCast((await locator.getRegistry())!);

        out.write("testing stringToProxy... ");
        let base = new Ice.ObjectPrx(communicator, "test @ TestAdapter");
        const base2 = new Ice.ObjectPrx(communicator, "test @ TestAdapter");
        const base3 = new Ice.ObjectPrx(communicator, "test");
        const base4 = new Ice.ObjectPrx(communicator, "ServerManager");
        const base5 = new Ice.ObjectPrx(communicator, "test2");
        const base6 = new Ice.ObjectPrx(communicator, "test @ ReplicatedAdapter");
        out.writeLine("ok");

        out.write("testing ice_locator and ice_getLocator... ");
        test(proxyIdentityCompare(base.ice_getLocator()!, communicator.getDefaultLocator()!));
        const anotherLocator = new Ice.LocatorPrx(communicator, "anotherLocator");
        base = base.ice_locator(anotherLocator);
        test(proxyIdentityCompare(base.ice_getLocator()!, anotherLocator));
        communicator.setDefaultLocator(null);
        base = new Ice.ObjectPrx(communicator, "test @ TestAdapter");
        test(base.ice_getLocator() === null);
        base = base.ice_locator(anotherLocator);
        test(proxyIdentityCompare(base.ice_getLocator()!, anotherLocator));
        communicator.setDefaultLocator(locator);
        base = new Ice.ObjectPrx(communicator, "test @ TestAdapter");
        test(proxyIdentityCompare(base.ice_getLocator()!, communicator.getDefaultLocator()!));

        // We also test ice_router/ice_getRouter (perhaps we should add a test/Ice/router test?)
        test(base.ice_getRouter() === null);
        const anotherRouter = new Ice.RouterPrx(communicator, "anotherRouter");
        base = base.ice_router(anotherRouter);
        test(proxyIdentityCompare(base.ice_getRouter()!, anotherRouter));
        const router = new Ice.RouterPrx(communicator, "dummyrouter");
        communicator.setDefaultRouter(router);
        base = new Ice.ObjectPrx(communicator, "test @ TestAdapter");
        test(proxyIdentityCompare(base.ice_getRouter()!, communicator.getDefaultRouter()!));
        communicator.setDefaultRouter(null);
        base = new Ice.ObjectPrx(communicator, "test @ TestAdapter");
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
        await obj.shutdown();
        await manager.startServer();
        await obj2.ice_ping();
        out.writeLine("ok");

        out.write("testing id@ReplicaGroupId indirect proxy... ");
        await obj.shutdown();
        await manager.startServer();
        await obj6.ice_ping();
        out.writeLine("ok");

        out.write("testing identity indirect proxy... ");
        await obj.shutdown();
        await manager.startServer();
        await obj3.ice_ping();
        await obj2.ice_ping();
        await obj.shutdown();
        await manager.startServer();
        await obj2.ice_ping();
        await obj3.ice_ping();
        await obj.shutdown();
        await manager.startServer();
        await obj2.ice_ping();
        await obj.shutdown();
        await manager.startServer();
        await obj3.ice_ping();
        await obj.shutdown();
        await manager.startServer();
        await obj2.ice_ping();
        await obj.shutdown();
        await manager.startServer();
        await obj5.ice_ping();
        out.writeLine("ok");

        out.write("testing proxy with unknown identity... ");
        base = new Ice.ObjectPrx(communicator, "unknown/unknown");
        try {
            await base.ice_ping();
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.NotRegisteredException) {
                test(ex.kindOfObject == "object");
                test(ex.id == "unknown/unknown");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("testing proxy with unknown adapter... ");
        base = new Ice.ObjectPrx(communicator, "test @ TestAdapterUnknown");
        try {
            await base.ice_ping();
        } catch (ex) {
            if (ex instanceof Ice.NotRegisteredException) {
                test(ex.kindOfObject == "object adapter");
                test(ex.id == "TestAdapterUnknown");
            } else {
                test(false, ex as Error);
            }
        }
        out.writeLine("ok");

        out.write("testing locator cache timeout... ");
        let count = await locator.getRequestCount();
        const basencc = new Ice.ObjectPrx(communicator, "test@TestAdapter").ice_connectionCached(false);
        await basencc.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == (await locator.getRequestCount()));
        await basencc.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == (await locator.getRequestCount()));
        await basencc.ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(count == (await locator.getRequestCount()));
        await Ice.Promise.delay(1200); // 1200ms
        await basencc.ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(++count == (await locator.getRequestCount()));

        await new Ice.ObjectPrx(communicator, "test").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        count += 2;
        test(count == (await locator.getRequestCount()));
        await new Ice.ObjectPrx(communicator, "test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        test(count == (await locator.getRequestCount()));
        await Ice.Promise.delay(1200); // 1200ms
        await new Ice.ObjectPrx(communicator, "test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        count += 2;
        test(count == (await locator.getRequestCount()));

        await new Ice.ObjectPrx(communicator, "test@TestAdapter").ice_locatorCacheTimeout(-1).ice_ping();
        test(count == (await locator.getRequestCount()));
        await new Ice.ObjectPrx(communicator, "test").ice_locatorCacheTimeout(-1).ice_ping();
        test(count == (await locator.getRequestCount()));
        await new Ice.ObjectPrx(communicator, "test@TestAdapter").ice_ping();
        test(count == (await locator.getRequestCount()));
        await new Ice.ObjectPrx(communicator, "test").ice_ping();
        test(count == (await locator.getRequestCount()));

        test(new Ice.ObjectPrx(communicator, "test").ice_locatorCacheTimeout(99).ice_getLocatorCacheTimeout() === 99);
        out.writeLine("ok");

        out.write("testing proxy from server... ");
        obj = new Test.TestIntfPrx(communicator, "test@TestAdapter");
        let hello = await obj.getHello();
        test(hello !== null);
        test(hello.ice_getAdapterId() == "TestAdapter");
        await hello.sayHello();
        hello = await obj.getReplicatedHello();
        test(hello !== null);
        test(hello.ice_getAdapterId() == "ReplicatedAdapter");
        await hello.sayHello();
        out.writeLine("ok");

        out.write("testing locator request queuing... ");
        hello = await obj.getReplicatedHello();
        test(hello !== null);
        hello = hello.ice_locatorCacheTimeout(0).ice_connectionCached(false);
        count = await locator.getRequestCount();
        await hello.ice_ping();
        test(++count == (await locator.getRequestCount()));

        let results: Promise<void>[] = [];
        for (let i = 0; i < 1000; i++) {
            results.push(
                hello.sayHello().catch(() => {
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
            await new Ice.ObjectPrx(communicator, "test@TestAdapter3").ice_ping();
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.NotRegisteredException) {
                test(ex.kindOfObject == "object adapter");
                test(ex.id == "TestAdapter3");
            } else {
                test(false, ex as Error);
            }
        }
        await registry.setAdapterDirectProxy("TestAdapter3", await locator.findAdapterById("TestAdapter"));
        try {
            await new Ice.ObjectPrx(communicator, "test@TestAdapter3").ice_ping();
            await registry.setAdapterDirectProxy("TestAdapter3", new Ice.ObjectPrx(communicator, "dummy:default"));
            await new Ice.ObjectPrx(communicator, "test@TestAdapter3").ice_ping();
        } catch (ex) {
            test(false, ex as Error);
        }

        try {
            await new Ice.ObjectPrx(communicator, "test@TestAdapter3").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex as Error);
        }
        try {
            await new Ice.ObjectPrx(communicator, "test@TestAdapter3").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex as Error);
        }
        await registry.setAdapterDirectProxy("TestAdapter3", await locator.findAdapterById("TestAdapter"));
        try {
            await new Ice.ObjectPrx(communicator, "test@TestAdapter3").ice_ping();
        } catch (ex) {
            test(false, ex as Error);
        }

        out.writeLine("ok");

        out.write("testing well-known object locator cache... ");
        await registry.addObject(new Ice.ObjectPrx(communicator, "test3@TestUnknown"));
        try {
            await new Ice.ObjectPrx(communicator, "test3").ice_ping();
            test(false);
        } catch (ex) {
            if (ex instanceof Ice.NotRegisteredException) {
                test(ex.kindOfObject == "object adapter");
                test(ex.id == "TestUnknown");
            } else {
                test(false, ex as Error);
            }
        }
        await registry.addObject(new Ice.ObjectPrx(communicator, "test3@TestAdapter4")); // Update
        await registry.setAdapterDirectProxy("TestAdapter4", new Ice.ObjectPrx(communicator, "dummy:default"));
        try {
            await new Ice.ObjectPrx(communicator, "test3").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex as Error);
        }

        await registry.setAdapterDirectProxy("TestAdapter4", await locator.findAdapterById("TestAdapter"));
        try {
            await new Ice.ObjectPrx(communicator, "test3").ice_ping();
        } catch (ex) {
            test(false, ex as Error);
        }

        await registry.setAdapterDirectProxy("TestAdapter4", new Ice.ObjectPrx(communicator, "dummy:default"));
        try {
            await new Ice.ObjectPrx(communicator, "test3").ice_ping();
        } catch (ex) {
            test(false, ex as Error);
        }

        try {
            await new Ice.ObjectPrx(communicator, "test@TestAdapter4").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex as Error);
        }

        try {
            await new Ice.ObjectPrx(communicator, "test@TestAdapter4").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex as Error);
        }

        try {
            await new Ice.ObjectPrx(communicator, "test3").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex as Error);
        }

        await registry.addObject(new Ice.ObjectPrx(communicator, "test3@TestAdapter"));
        try {
            await new Ice.ObjectPrx(communicator, "test3").ice_ping();
        } catch (ex) {
            test(false, ex as Error);
        }

        await registry.addObject(new Ice.ObjectPrx(communicator, "test4"));
        try {
            await new Ice.ObjectPrx(communicator, "test4").ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.NoEndpointException, ex as Error);
        }
        out.writeLine("ok");

        out.write("testing locator cache background updates... ");
        {
            const initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().clone();
            initData.properties.setProperty("Ice.BackgroundLocatorCacheUpdates", "1");
            const ic = Ice.initialize(initData);

            await registry.setAdapterDirectProxy("TestAdapter5", await locator.findAdapterById("TestAdapter"));
            await registry.addObject(new Ice.ObjectPrx(communicator, "test3@TestAdapter"));

            count = await locator.getRequestCount();
            await new Ice.ObjectPrx(ic, "test@TestAdapter5").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
            await new Ice.ObjectPrx(ic, "test3").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
            count += 3;
            test(count == (await locator.getRequestCount()));
            await registry.setAdapterDirectProxy("TestAdapter5", null);
            await registry.addObject(new Ice.ObjectPrx(communicator, "test3:default"));
            await new Ice.ObjectPrx(ic, "test@TestAdapter5").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
            await new Ice.ObjectPrx(ic, "test3").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
            test(count == (await locator.getRequestCount()));
            await Ice.Promise.delay(1200);

            // The following request should trigger the background
            // updates but still use the cached endpoints and
            // therefore succeed.
            await new Ice.ObjectPrx(ic, "test@TestAdapter5").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
            await new Ice.ObjectPrx(ic, "test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.

            try {
                while (true) {
                    await new Ice.ObjectPrx(ic, "test@TestAdapter5").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                    await Ice.Promise.delay(10);
                }
            } catch (ex) {
                // Expected to fail once they endpoints have been updated in the background.
                test(ex instanceof Ice.LocalException, ex as Error);
            }
            try {
                while (true) {
                    await new Ice.ObjectPrx(ic, "test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                    await Ice.Promise.delay(10);
                }
            } catch (ex) {
                // Expected to fail once they endpoints have been updated in the background.
                test(ex instanceof Ice.LocalException, ex as Error);
            }
            await ic.destroy();
        }
        out.writeLine("ok");

        out.write("testing proxy from server after shutdown... ");
        hello = await obj.getReplicatedHello();
        test(hello !== null);
        await obj.shutdown();
        await manager.startServer();
        await hello.sayHello();
        out.writeLine("ok");

        out.write("testing object migration... ");
        hello = await Test.HelloPrx.checkedCast(new Ice.ObjectPrx(communicator, "hello"));
        test(hello !== null);
        await obj.migrateHello();
        const conn = await hello.ice_getConnection();
        await conn.close();
        await hello.sayHello();
        await obj.migrateHello();
        await hello.sayHello();
        await obj.migrateHello();
        await hello.sayHello();
        out.writeLine("ok");

        out.write("testing locator encoding resolution... ");
        hello = await Test.HelloPrx.checkedCast(new Ice.ObjectPrx(communicator, "hello"));
        count = await locator.getRequestCount();
        await new Ice.ObjectPrx(communicator, "test@TestAdapter").ice_encodingVersion(Ice.Encoding_1_1).ice_ping();
        test(count == (await locator.getRequestCount()));
        await new Ice.ObjectPrx(communicator, "test@TestAdapter10").ice_encodingVersion(Ice.Encoding_1_0).ice_ping();
        test(++count == (await locator.getRequestCount()));
        await new Ice.ObjectPrx(communicator, "test -e 1.0@TestAdapter10-2").ice_ping();
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
            properties.setProperty("Ice.Default.Locator", `locator:${this.getTestEndpoint(properties)}`);
            [communicator] = this.initialize(properties);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
