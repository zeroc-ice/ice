//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

// Verify that the client idle check does not abort the connection while dispatching a request.
// The server idle timeout is set to 1 second, and the server idle check is disabled.
// The client idle timeout is also set to 1 second, with the idle check enabled.
// The client sleeps for 2 seconds while dispatching the request.
// The client continues processing heartbeats while dispatching the request, preventing the connection from being aborted.
async function testConnectionNotAbortedByIdleCheck(properties: Ice.Properties, helper: TestHelper): Promise<void> {
    const output = helper.getWriter();

    output.write("testing connection not aborted by idle check... ");

    // The server has 1s idle timeout, and disabled idle check.
    const proxyString = `test-bidir: ${helper.getTestEndpoint(4)}`;

    const completedPromise = new Ice.Promise<void>();

    const TestI = class extends Test.DelayedTestIntf {
        async sleep(msecs: number, current: Ice.Current): Promise<void> {
            await Ice.Promise.delay(msecs);
            completedPromise.resolve();
        }
    };

    // Create a new communicator with the desired properties.
    properties = properties.clone();
    properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
    properties.setProperty("Ice.Connection.Client.EnableIdleCheck", "1");
    properties.setProperty("Ice.Warn.Connections", "0");
    const initData = new Ice.InitializationData();
    initData.properties = properties;
    const communicator = Ice.initialize(initData);
    try {
        const adapter = await communicator.createObjectAdapter("");
        const callback = new Test.TestIntfPrx(adapter.add(new TestI(), Ice.stringToIdentity("test")));

        const p = new Test.TestIntfBidirPrx(communicator, proxyString);

        const connection = await p.ice_getConnection();
        test(connection != null);
        connection.setAdapter(adapter);

        await p.ice_oneway().makeSleep(false, 2000, callback);
        await completedPromise;
    } finally {
        await communicator.destroy();
    }
    output.writeLine("ok");
}

// Verify that the client idle check aborts the connection while dispatching a request.
// The server idle timeout is misconfigured and set to 3 seconds, with the server idle check disabled.
// The client idle timeout is set to 1 second, with the idle check enabled.
// The client sleeps for 2 seconds while dispatching the request.
// The client will not receive a heartbeat in time while dispatching the request, causing the connection to be aborted.
async function testConnectionAbortedByIdleCheck(properties: Ice.Properties, helper: TestHelper): Promise<void> {
    const output = helper.getWriter();

    output.write("testing connection aborted by idle check... ");

    // The server has 3s idle timeout, and disabled idle check.
    const proxyString = `test-bidir: ${helper.getTestEndpoint(5)}`;

    const completedPromise = new Ice.Promise<void>();

    const TestI = class extends Test.DelayedTestIntf {
        async sleep(msecs: number, current: Ice.Current): Promise<void> {
            await Ice.Promise.delay(msecs);
            completedPromise.resolve();
        }
    };

    // Create a new communicator with the desired properties.
    properties = properties.clone();
    properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
    properties.setProperty("Ice.Connection.Client.EnableIdleCheck", "1");
    properties.setProperty("Ice.Warn.Connections", "0");
    const initData = new Ice.InitializationData();
    initData.properties = properties;
    const communicator = Ice.initialize(initData);
    try {
        const adapter = await communicator.createObjectAdapter("");
        const obj = adapter.add(new TestI(), Ice.stringToIdentity("test"));
        const callback = Test.TestIntfPrx.uncheckedCast(obj);

        const p = Test.TestIntfBidirPrx.uncheckedCast(communicator.stringToProxy(proxyString));

        const connection = await p.ice_getConnection();
        test(connection != null);
        connection.setAdapter(adapter);

        await p.ice_oneway().makeSleep(true, 2000, callback);
        await completedPromise;
    } finally {
        await communicator.destroy();
    }
    output.writeLine("ok");
}

// Verifies the behavior with the server idle check enabled or disabled when the client and the server have mismatched idle
// timeouts (here: 3s on the server side and 1s on the client side).
async function testServerWithEnableDisableIdleCheck(
    enabled: boolean,
    properties: Ice.Properties,
    helper: TestHelper,
): Promise<void> {
    const output = helper.getWriter();
    const enabledString = enabled ? "enabled" : "disabled";
    output.write(`testing with server idle check ${enabledString}... `);

    // The server has 3s idle timeout, and enabled idle check.
    const proxyString3s = `test: ${helper.getTestEndpoint(2)}`;

    // Create a new communicator with the desired properties.
    properties = properties.clone();
    properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
    properties.setProperty("Ice.Connection.Client.EnableIdleCheck", enabled ? "1" : "0");
    properties.setProperty("Ice.Warn.Connections", "0");
    const initData = new Ice.InitializationData();
    initData.properties = properties;
    const communicator = Ice.initialize(initData);
    const p = new Test.TestIntfPrx(communicator, proxyString3s);
    try {
        const connection = await p.ice_getConnection();
        test(connection != null);

        try {
            await p.sleep(2000); // the implementation in the server sleeps for 2,000ms
            test(!enabled);
        } catch (ex) {
            test(ex instanceof Ice.ConnectionAbortedException);
            test(enabled);
        }
    } finally {
        await communicator.destroy();
    }
    output.writeLine("ok");
}

// Verifies the idle check is disabled when the idle timeout is set to 0.
async function testNoIdleTimeout(properties: Ice.Properties, helper: TestHelper): Promise<void> {
    const output = helper.getWriter();
    output.write(`testing connection with idle timeout set to 0... `);

    const proxyStringNoIdleTimeout = `test: ${helper.getTestEndpoint(3)}`;

    // Create a new communicator with the desired properties.
    properties = properties.clone();
    properties.setProperty("Ice.Connection.Client.IdleTimeout", "0");
    const initData = new Ice.InitializationData();
    initData.properties = properties;
    const communicator = Ice.initialize(initData);
    const p = new Test.TestIntfPrx(communicator, proxyStringNoIdleTimeout);
    try {
        const connection = await p.ice_getConnection();
        test(connection != null);
        await p.sleep(2000); // the implementation in the server sleeps for 2,000ms
        await connection.close();
    } finally {
        await communicator.destroy();
    }
    output.writeLine("ok");
}

export class Client extends TestHelper {
    async allTests(): Promise<void> {
        const communicator = this.communicator();

        await testConnectionAbortedByIdleCheck(communicator.getProperties(), this);
        await testConnectionNotAbortedByIdleCheck(communicator.getProperties(), this);

        await testServerWithEnableDisableIdleCheck(true, communicator.getProperties(), this);
        await testServerWithEnableDisableIdleCheck(false, communicator.getProperties(), this);

        await testNoIdleTimeout(communicator.getProperties(), this);

        const p = Test.TestIntfPrx.uncheckedCast(communicator.stringToProxy(`test: ${this.getTestEndpoint(0)}`));
        await p.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
            [communicator] = this.initialize(properties);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
