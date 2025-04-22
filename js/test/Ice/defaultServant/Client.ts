// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { TestHelper } from "../../Common/TestHelper.js";
import { Test } from "./Test.js";
import { MyObjectI } from "./MyObjectI.js";
const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests(args: string[], echo: Test.EchoPrx) {
        const communicator = this.communicator();

        const adapter = await communicator.createObjectAdapter("");
        await echo.setConnection();
        echo.ice_getCachedConnection().setAdapter(adapter);

        const out = this.getWriter();

        const servant = new MyObjectI();
        adapter.addDefaultServant(servant, "foo");

        out.write("testing single category... ");

        test(adapter.findDefaultServant("foo") === servant);
        test(adapter.findDefaultServant("bar") === null);

        const names = ["foo", "bar", "x", "y", "abcdefg"];
        let prx: Test.MyObjectPrx;
        for (const name of names) {
            prx = new Test.MyObjectPrx(communicator, `foo/${name}:${this.getTestEndpoint()}`);
            await prx.ice_ping();
            test((await prx.getName()) == name);
        }

        prx = new Test.MyObjectPrx(communicator, `foo/ObjectNotExist:${this.getTestEndpoint()}`);
        try {
            await prx.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.ObjectNotExistException);
        }

        try {
            await prx.getName();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.ObjectNotExistException);
        }

        prx = new Test.MyObjectPrx(communicator, `foo/FacetNotExist:${this.getTestEndpoint()}`);
        try {
            await prx.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.FacetNotExistException);
        }

        try {
            await prx.getName();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.FacetNotExistException);
        }

        for (const name of names) {
            prx = new Test.MyObjectPrx(communicator, `bar/${name}:${this.getTestEndpoint()}`);
            try {
                await prx.ice_ping();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.ObjectNotExistException);
            }

            try {
                await prx.getName();
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.ObjectNotExistException);
            }
        }

        adapter.removeDefaultServant("foo");
        prx = new Test.MyObjectPrx(communicator, `foo/x:${this.getTestEndpoint()}`);
        try {
            await prx.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.ObjectNotExistException);
        }
        out.writeLine("ok");

        out.write("testing default category... ");
        adapter.addDefaultServant(servant, "");

        test(adapter.findDefaultServant("bar") === null);
        test(adapter.findDefaultServant("") == servant);

        for (const name of names) {
            prx = new Test.MyObjectPrx(communicator, `bar/${name}:${this.getTestEndpoint()}`);
            await prx.ice_ping();
            test((await prx.getName()) == name);
        }
        out.writeLine("ok");
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        let echo: Test.EchoPrx | null = null;
        try {
            [communicator, args] = this.initialize(args);
            echo = new Test.EchoPrx(communicator, `__echo:${this.getTestEndpoint()}`);
            await this.allTests(args, echo);
        } finally {
            if (echo) {
                try {
                    await echo.shutdown();
                } catch (ex) {}
            }
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
