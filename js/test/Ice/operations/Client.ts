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

        await cl.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.BatchAutoFlushSize", "100");
            properties.setProperty("Ice.Trace.Protocol", "1");
            properties.setProperty("Ice.Trace.Network", "3");
            [communicator] = this.initialize(properties);
            await this.allTests(false);
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
