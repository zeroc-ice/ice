// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests(communicator: Ice.Communicator, communicator2: Ice.Communicator) {
        const out = this.getWriter();

        const retry1 = new Test.RetryPrx(communicator, `retry:${this.getTestEndpoint()}`);
        let retry2 = new Test.RetryPrx(communicator, `retry:${this.getTestEndpoint()}`);

        out.write("calling regular operation with first proxy... ");
        await retry1.op(false);
        out.writeLine("ok");

        out.write("calling operation to kill connection with second proxy... ");
        try {
            await retry2.op(true);
            test(false);
        } catch (ex) {
            if (TestHelper.isBrowser()) {
                test(ex instanceof Ice.SocketException, ex);
            } else {
                // Nodejs
                test(ex instanceof Ice.ConnectionLostException, ex);
            }
            out.writeLine("ok");
        }

        out.write("calling regular operation with first proxy again... ");
        await retry1.op(false);
        out.writeLine("ok");

        out.write("testing idempotent operation... ");
        const count = await retry1.opIdempotent(4);
        test(count === 4);
        out.writeLine("ok");

        out.write("testing non-idempotent operation... ");
        try {
            await retry1.opNotIdempotent();
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.LocalException, ex);
        }
        out.writeLine("ok");

        out.write("testing invocation timeout and retries... ");
        retry2 = new Test.RetryPrx(communicator2, retry1.toString());
        retry2.ice_getConnection();
        try {
            await retry2.ice_invocationTimeout(500).opIdempotent(4);
            test(false);
        } catch (ex) {
            test(ex instanceof Ice.InvocationTimeoutException, ex);
        }

        await retry2.opIdempotent(-1);
        out.writeLine("ok");

        await retry1.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        let communicator2: Ice.Communicator | null = null;
        try {
            let [properties] = this.createTestProperties(args);

            //
            // For this test, we want to disable retries.
            //
            properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

            //
            // We don't want connection warnings because of the timeout
            //
            properties.setProperty("Ice.Warn.Connections", "0");

            [communicator] = this.initialize(properties);

            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            properties = communicator.getProperties().clone();
            properties.setProperty("Ice.RetryIntervals", "0 1 10000");
            [communicator2] = this.initialize(properties);

            await this.allTests(communicator, communicator2);
        } finally {
            if (communicator2) {
                await communicator2.destroy();
            }

            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
