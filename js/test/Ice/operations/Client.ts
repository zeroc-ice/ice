// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

import { twoways } from "./Twoways.js";
import { oneways } from "./Oneways.js";
import { batchOneways } from "./BatchOneways.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests(bidir: boolean) {
        const out = this.getWriter();
        const communicator = this.communicator();
        out.write("testing twoway operations... ");
        const cl = new Test.MyClassPrx(communicator, `test:${this.getTestEndpoint()}`);
        const derived = new Test.MyDerivedClassPrx(cl);

        await twoways(communicator, cl!, bidir, this);
        await twoways(communicator, derived!, bidir, this);
        out.writeLine("ok");

        out.write("testing oneway operations... ");
        await oneways(cl);
        out.writeLine("ok");

        out.write("testing batch oneway operations... ");
        await batchOneways(cl);
        out.writeLine("ok");

        out.write("testing server shutdown... ");
        await cl.shutdown();
        try {
            await cl.ice_invocationTimeout(100).ice_ping(); // Use timeout to speed up testing on Windows
            throw new Error("test failed");
        } catch (ex) {
            if (ex instanceof Ice.LocalException) {
                out.writeLine("ok");
            } else {
                throw ex;
            }
        }
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.BatchAutoFlushSize", "100");
            [communicator] = this.initialize(properties);
            await this.allTests(false);
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
