//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests() {
        const out = this.getWriter();
        const communicator = this.communicator();

        const obj = new Test.TestIntfPrx(communicator, `test:${this.getTestEndpoint()}`);
        {
            out.write("creating/destroying/recreating object adapter... ");
            communicator.getProperties().setProperty("TransientTestAdapter.ProxyOptions", "-t");
            let adapter = await communicator.createObjectAdapter("TransientTestAdapter");
            try {
                await communicator.createObjectAdapter("TransientTestAdapter");
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.AlreadyRegisteredException);
            }
            adapter.destroy();
            out.writeLine("ok");
        }

        out.write("creating/activating/deactivating object adapter in one operation... ");
        await obj.transient();
        out.writeLine("ok");

        out.write("testing connection closure... ");
        for (let i = 0; i < 10; ++i) {
            const initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().clone();
            const comm = Ice.initialize(initData);
            await comm.stringToProxy("test:" + this.getTestEndpoint()).ice_ping();
            await comm.destroy();
        }
        out.writeLine("ok");

        out.write("testing object adapter published endpoints... ");
        {
            communicator
                .getProperties()
                .setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
            const adapter = await communicator.createObjectAdapter("PAdapter");
            test(adapter.getPublishedEndpoints().length === 1);
            const endpt = adapter.getPublishedEndpoints()[0];
            test(endpt.toString() == "tcp -h localhost -p 12345 -t 30000");
            const prx = communicator.stringToProxy(
                "dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000",
            );
            adapter.setPublishedEndpoints(prx.ice_getEndpoints());
            test(adapter.getPublishedEndpoints().length === 2);
            const id = new Ice.Identity();
            id.name = "dummy";
            test(Ice.ArrayUtil.equals(adapter.createProxy(id).ice_getEndpoints(), prx.ice_getEndpoints()));
            test(Ice.ArrayUtil.equals(adapter.getPublishedEndpoints(), prx.ice_getEndpoints()));
            adapter.destroy();
            test(adapter.getPublishedEndpoints().length === 0);
        }
        out.writeLine("ok");

        test(obj!.ice_getConnection() !== null);
        {
            out.write("testing object adapter with bi-dir connection... ");

            test(communicator.getDefaultObjectAdapter() === null);
            test(obj.ice_getCachedConnection().getAdapter() === null);

            let adapter = await communicator.createObjectAdapter("");

            communicator.setDefaultObjectAdapter(adapter);
            test(communicator.getDefaultObjectAdapter() === adapter);

            // create new connection
            await obj.ice_getCachedConnection().close();
            await obj.ice_ping();

            test(obj.ice_getCachedConnection().getAdapter() === adapter);

            // Ensure destroying the OA doesn't affect the ability to send outgoing requests.
            adapter.destroy();
            await obj.ice_getCachedConnection().close();
            await obj.ice_ping();

            communicator.setDefaultObjectAdapter(null);

            // create new connection
            await obj.ice_getCachedConnection().close();
            await obj.ice_ping();
            test(obj.ice_getCachedConnection().getAdapter() === null);

            adapter = await communicator.createObjectAdapter("");
            obj.ice_getCachedConnection().setAdapter(adapter);
            test(obj.ice_getCachedConnection().getAdapter() === adapter);
            obj.ice_getCachedConnection().setAdapter(null);

            adapter.destroy();
            try {
                (await obj!.ice_getConnection()).setAdapter(adapter);
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.ObjectAdapterDestroyedException);
            }
            out.writeLine("ok");
        }

        out.write("testing object adapter with router... ");
        {
            const routerId = new Ice.Identity();
            routerId.name = "router";
            let router = Ice.RouterPrx.uncheckedCast(obj.ice_identity(routerId).ice_connectionId("rc"));
            const adapter = await communicator.createObjectAdapterWithRouter("", router);
            test(adapter.getPublishedEndpoints().length == 1);
            test(adapter.getPublishedEndpoints()[0].toString() == "tcp -h localhost -p 23456 -t 30000");
            try {
                adapter.setPublishedEndpoints(router.ice_getEndpoints());
                test(false);
            } catch (ex) {
                // Expected.
                test(ex instanceof Error);
            }
            adapter.destroy();

            try {
                routerId.name = "test";
                router = new Ice.RouterPrx(communicator, `test:${this.getTestEndpoint(0)}`);
                await communicator.createObjectAdapterWithRouter("", router);
                test(false);
            } catch (ex) {
                // Expected: the "test" object doesn't implement Ice::Router!
                test(ex instanceof Ice.OperationNotExistException);
            }

            try {
                router = new Ice.RouterPrx(communicator, `router:${this.getTestEndpoint(1)}`);
                await communicator.createObjectAdapterWithRouter("", router);
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.ConnectFailedException || ex instanceof Ice.ConnectTimeoutException);
            }
        }
        out.writeLine("ok");

        out.write("deactivating object adapter in the server... ");
        await obj!.deactivate();
        out.writeLine("ok");

        out.write("testing whether server is gone... ");
        try {
            await obj!.ice_invocationTimeout(100).ice_ping(); // Use timeout to speed up testing on Windows
            throw new Error();
        } catch (ex) {
            test(ex instanceof Ice.LocalException);
            out.writeLine("ok");
        }
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            [communicator, args] = this.initialize(args);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
