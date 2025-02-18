// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests() {
        const controller = new Test.ControllerPrx(this.communicator(), `controller:${this.getTestEndpoint(1)}`);
        test(controller !== null);
        try {
            await this.allTestsWithController(controller);
        } catch (ex) {
            // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent the test
            // from hanging on exit in case a connection which disables timeouts is still opened.
            controller.resumeAdapter();
            throw ex;
        }
    }

    async allTestsWithController(controller: Test.ControllerPrx) {
        const communicator = this.communicator();
        const out = this.getWriter();

        const timeout = new Test.TimeoutPrx(communicator, `timeout: ${this.getTestEndpoint()}`);

        out.write("testing connect timeout... ");
        {
            await controller.holdAdapter(-1);
            try {
                await timeout.op(); // Expect ConnectTimeoutException.
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.ConnectTimeoutException, ex);
            }
            controller.resumeAdapter();
            await timeout.op(); // Ensure adapter is active.
        }

        {
            const properties = communicator.getProperties().clone();
            properties.setProperty("Ice.Connection.Client.ConnectTimeout", "-1");
            const [communicator2, _] = this.initialize(properties);

            const to = new Test.TimeoutPrx(communicator2, timeout.toString());
            controller.holdAdapter(100);
            try {
                await to.op(); // Expect success.
            } catch (ex) {
                test(false, ex);
            } finally {
                await controller.resumeAdapter();
                await communicator2.destroy();
            }
        }
        out.writeLine("ok");

        out.write("testing invocation timeout... ");
        {
            const connection = await timeout.ice_getConnection();
            let to = timeout.ice_invocationTimeout(100);
            test(connection == (await to.ice_getConnection()));

            try {
                await to.sleep(1000);
                test(false);
            } catch (ex) {
                test(ex instanceof Ice.InvocationTimeoutException, ex);
            }
            await timeout.ice_ping();
            to = timeout.ice_invocationTimeout(1000);
            test(connection === (await timeout.ice_getConnection()));

            try {
                await to.sleep(100);
            } catch (ex) {
                test(false);
            }
        }
        out.writeLine("ok");

        out.write("testing close timeout... ");
        {
            try {
                const connection = await timeout.ice_getConnection();
                await controller.holdAdapter(-1);
                const closed = connection.close();

                try {
                    connection.getInfo(); // getInfo() doesn't throw in the closing state
                } catch (ex) {
                    test(false, ex);
                }

                try {
                    await closed;
                    test(false);
                } catch (ex) {
                    test(ex instanceof Ice.CloseTimeoutException, ex);
                }

                try {
                    connection.getInfo();
                } catch (ex) {
                    test(ex instanceof Ice.CloseTimeoutException, ex);
                }
            } finally {
                await controller.resumeAdapter();
            }
        }
        controller.shutdown();
        out.writeLine("ok");
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            //
            // For this test, we want to disable retries.
            //
            properties.setProperty("Ice.RetryIntervals", "-1");

            properties.setProperty("Ice.Connection.Client.ConnectTimeout", "1");
            properties.setProperty("Ice.Connection.Client.CloseTimeout", "1");

            [communicator] = this.initialize(properties);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
