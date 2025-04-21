// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { TestHelper } from "../../Common/TestHelper.js";
import { Test } from "./Test.js";
const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests(args: string[]) {
        const communicator = this.communicator();
        const oa = new Test.RemoteObjectAdapterPrx(communicator, `remote-object-adapter:${this.getTestEndpoint()}`);
        const out = this.getWriter();

        // Ask the server to register the default servant for the "foo"
        await oa.addDefaultServant("foo");

        out.write("testing single category... ");

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

        await oa.removeDefaultServant("foo");
        prx = new Test.MyObjectPrx(communicator, `foo/x:${this.getTestEndpoint()}`);
        try {
            await prx.ice_ping();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.ObjectNotExistException);
        }
        out.writeLine("ok");

        out.write("testing default category... ");
        await oa.addDefaultServant("");
        for (const name of names) {
            prx = new Test.MyObjectPrx(communicator, `bar/${name}:${this.getTestEndpoint()}`);
            await prx.ice_ping();
            test((await prx.getName()) == name);
        }
        out.writeLine("ok");

        await oa.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            [communicator, args] = this.initialize(args);
            await this.allTests(args);
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
