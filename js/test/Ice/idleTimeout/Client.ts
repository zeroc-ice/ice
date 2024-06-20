//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

// We verify that the idle check aborts the connection when the connection (here client connection) remains idle for
// longer than idle timeout. Here, the client has an idle timeout of 1s and idle checks enabled. We intentionally
// misconfigure the server with an idle timeout of 3s to send heartbeats every 1.5s, which is too long to prevent
// the client from aborting the connection.
async function testConnectionAbortedByIdleCheck(
    proxyString: string,
    properties: Ice.Properties,
    helper: TestHelper,
): Promise<void> {
    const output = helper.getWriter();
    output.write("testing that the idle check aborts a connection that does not receive anything for 1s... ");

    // Create a new communicator with the desired properties.
    properties = properties.clone();
    properties.setProperty("Ice.Connection.IdleTimeout", "1");
    properties.setProperty("Ice.Connection.EnableIdleCheck", "1");
    properties.setProperty("Ice.Warn.Connections", "0");
    const initData = new Ice.InitializationData();
    initData.properties = properties;
    const communicator = Ice.initialize(initData);
    const p = Test.TestIntfPrx.uncheckedCast(communicator.stringToProxy(proxyString));

    // Establish connection.
    const connection = await p.ice_getConnection();
    test(connection !== null);

    // The idle check on the server side aborts the connection because it doesn't get a heartbeat in a timely fashion.
    try {
        await p.sleep(2000); // the implementation in the server sleeps for 2,000ms
        test(false); // we expect the server to abort the connection after about 1 second.
    } catch (ConnectionLostException) {
        // Expected
    }
    output.writeLine("ok");
}

// Verifies the behavior with the idle check enabled or disabled when the client and the server have mismatched idle
// timeouts (here: 1s on the server side and 3s on the client side).
async function testEnableDisableIdleCheck(
    enabled: boolean,
    proxyString: string,
    properties: Ice.Properties,
    helper: TestHelper,
): Promise<void> {
    const output = helper.getWriter();
    const enabledString = enabled ? "enabled" : "disabled";
    output.write(`testing connection with idle check ${enabledString}... `);

    // Create a new communicator with the desired properties.
    properties = properties.clone();
    properties.setProperty("Ice.Connection.IdleTimeout", "3");
    properties.setProperty("Ice.Connection.EnableIdleCheck", enabled ? "1" : "0");
    properties.setProperty("Ice.Warn.Connections", "0");
    const initData = new Ice.InitializationData();
    initData.properties = properties;
    const communicator = Ice.initialize(initData);
    const p = Test.TestIntfPrx.uncheckedCast(communicator.stringToProxy(proxyString));

    const connection = await p.ice_getConnection();
    test(connection != null);

    try {
        await p.sleep(2000); // the implementation in the server sleeps for 2,000ms
        test(!enabled);
    } catch (ex) {
        test(ex instanceof Ice.ConnectionTimeoutException); // TODO: should be ConnectionIdleException
        test(enabled);
    }
    output.writeLine("ok");
}

export class Client extends TestHelper {
    async allTests(): Promise<void> {
        const communicator = this.communicator();
        const proxyString = `test: ${this.getTestEndpoint(2)}`;
        const p = Test.TestIntfPrx.uncheckedCast(communicator.stringToProxy(proxyString));

        await testConnectionAbortedByIdleCheck(proxyString, communicator.getProperties(), this);
        // await testEnableDisableIdleCheck(true, proxyString, communicator.getProperties(), this);
        // await testEnableDisableIdleCheck(false, proxyString, communicator.getProperties(), this);

        await p.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Connection.IdleTimeout", "1");
            [communicator] = this.initialize(properties);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
