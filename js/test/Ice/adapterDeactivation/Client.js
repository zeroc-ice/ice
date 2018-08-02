// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestHelper = require("TestHelper").TestHelper;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests()
        {
            const out = this.getWriter();
            const communicator = this.communicator();

            out.write("testing stringToProxy... ");
            const ref = "test:" + this.getTestEndpoint();
            const base = communicator.stringToProxy(ref);
            test(base !== null);
            out.writeLine("ok");

            out.write("testing checked cast... ");
            const obj = await Test.TestIntfPrx.checkedCast(base);
            test(obj !== null);
            test(obj.equals(base));
            out.writeLine("ok");

            {
                out.write("creating/destroying/recreating object adapter... ");
                communicator.getProperties().setProperty("TransientTestAdapter.AdapterId", "dummy");
                let adapter = await communicator.createObjectAdapter("TransientTestAdapter");
                try
                {
                    await communicator.createObjectAdapterWithEndpoints("TransientTestAdapter");
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.AlreadyRegisteredException);
                }
                await adapter.destroy();

                //
                // Use a different port than the first adapter to avoid an "address already in use" error.
                //
                adapter = await communicator.createObjectAdapterWithEndpoints("TransientTestAdapter");
                await adapter.destroy();
                out.writeLine("ok");
            }

            out.write("creating/activating/deactivating object adapter in one operation... ");
            await obj.transient();
            out.writeLine("ok");

            out.write("testing connection closure... ");
            for(let i = 0; i < 10; ++i)
            {
                const initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().clone();
                const comm = Ice.initialize(initData);
                comm.stringToProxy("test:" + this.getTestEndpoint()).ice_ping().catch(ex => {});
                await comm.destroy();
            }
            out.writeLine("ok");

            out.write("testing object adapter published endpoints... ");
            {
                communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
                const adapter = await communicator.createObjectAdapter("PAdapter");
                test(adapter.getPublishedEndpoints().length === 1);
                const endpt = adapter.getPublishedEndpoints()[0];
                test(endpt.toString() == "tcp -h localhost -p 12345 -t 30000");
                const prx =
                      communicator.stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
                adapter.setPublishedEndpoints(prx.ice_getEndpoints());
                test(adapter.getPublishedEndpoints().length === 2);
                const id = new Ice.Identity();
                id.name = "dummy";
                test(Ice.ArrayUtil.equals(adapter.createProxy(id).ice_getEndpoints(), prx.ice_getEndpoints()));
                test(Ice.ArrayUtil.equals(adapter.getPublishedEndpoints(), prx.ice_getEndpoints()));
                await adapter.refreshPublishedEndpoints();
                test(adapter.getPublishedEndpoints().length === 1);
                test(adapter.getPublishedEndpoints()[0].equals(endpt));
                communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
                await adapter.refreshPublishedEndpoints();
                test(adapter.getPublishedEndpoints().length === 1);
                test(adapter.getPublishedEndpoints()[0].toString() == "tcp -h localhost -p 12345 -t 20000");
                await adapter.destroy();
                test(adapter.getPublishedEndpoints().length === 0);
            }
            out.writeLine("ok");

            test(obj.ice_getConnection() !== null);
            {
                out.write("testing object adapter with bi-dir connection... ");
                const adapter = await communicator.createObjectAdapter("");
                (await obj.ice_getConnection()).setAdapter(adapter);
                (await obj.ice_getConnection()).setAdapter(null);
                await adapter.deactivate();
                try
                {
                    (await obj.ice_getConnection()).setAdapter(adapter);
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.ObjectAdapterDeactivatedException);
                }
                out.writeLine("ok");
            }

            out.write("testing object adapter with router... ");
            {
                const routerId = new Ice.Identity();
                routerId.name = "router";
                let router = Ice.RouterPrx.uncheckedCast(base.ice_identity(routerId).ice_connectionId("rc"));
                const adapter = await communicator.createObjectAdapterWithRouter("", router);
                test(adapter.getPublishedEndpoints().length == 1);
                test(adapter.getPublishedEndpoints()[0].toString() == "tcp -h localhost -p 23456 -t 30000");
                await adapter.refreshPublishedEndpoints();
                test(adapter.getPublishedEndpoints().length == 1);
                test(adapter.getPublishedEndpoints()[0].toString() == "tcp -h localhost -p 23457 -t 30000");
                try
                {
                    adapter.setPublishedEndpoints(router.ice_getEndpoints());
                    test(false);
                }
                catch(ex)
                {
                    // Expected.
                    test(ex instanceof Error);
                }
                await adapter.destroy();

                try
                {
                    routerId.name = "test";
                    router = Ice.RouterPrx.uncheckedCast(base.ice_identity(routerId));
                    await communicator.createObjectAdapterWithRouter("", router);
                    test(false);
                }
                catch(ex)
                {
                    // Expected: the "test" object doesn't implement Ice::Router!
                    test(ex instanceof Ice.OperationNotExistException);
                }

                try
                {
                    router = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("router:" + this.getTestEndpoint(1)));
                    await communicator.createObjectAdapterWithRouter("", router);
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.ConnectFailedException);
                }
            }
            out.writeLine("ok");

            out.write("deactivating object adapter in the server... ");
            await obj.deactivate();
            out.writeLine("ok");

            out.write("testing adapter states... ");
            {
                const adpt = await communicator.createObjectAdapter("");
                test(!adpt.isDeactivated());
                await adpt.activate();
                test(!adpt.isDeactivated());

                let isHolding = false;
                adpt.waitForHold().then(() =>
                                        {
                                            isHolding = true;
                                        });
                test(!isHolding);
                adpt.hold();
                await adpt.waitForHold();
                await Ice.Promise.delay(10); // Relinquish the thread to allow the continuation to execute.
                test(isHolding);

                isHolding = false;
                adpt.waitForHold().then(() =>
                                        {
                                            isHolding = true;
                                        });

                let isDeactivated = false;
                adpt.waitForDeactivate().then(() =>
                                              {
                                                  isDeactivated = true;
                                              });
                test(!isDeactivated);
                await adpt.deactivate();
                await adpt.waitForDeactivate();
                await Ice.Promise.delay(10); // Relinquish the thread to allow the continuation to execute.
                test(isDeactivated && isHolding);
                test(adpt.isDeactivated());
                await adpt.destroy();
            }
            out.writeLine("ok");

            out.write("testing whether server is gone... ");
            try
            {
                await obj.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
                throw new Error();
            }
            catch(ex)
            {
                test(ex instanceof Ice.LocalException);
                out.writeLine("ok");
            }
        }

        async run(args)
        {
            let communicator;
            try
            {
                communicator = this.initialize(args);
                await this.allTests();
            }
            finally
            {
                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Client = Client;

}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require : this.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports : this));
