// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

async function testClientInactivityTimeout(p: Test.TestIntfPrx, helper: TestHelper) {
    const output = helper.getWriter();
    output.write("testing that the client side inactivity timeout shuts down the connection... ");

    await p.ice_ping();
    const connection = await p.ice_getConnection();
    test(connection !== null);

    // The inactivity timeout is 3s on the client side and 5s on the server side. 4 seconds tests the client side.
    await Ice.Promise.delay(4000);
    await p.ice_ping();
    const connection2 = await p.ice_getConnection();
    test(connection2 != connection);
    output.writeLine("ok");
}

async function testWithOutstandingRequest(p: Test.TestIntfPrx, oneway: boolean, helper: TestHelper) {
    const output = helper.getWriter();
    const onewayString = oneway ? "one-way" : "two-way";
    output.write(`testing the inactivity timeout with an outstanding ${onewayString} request... `);

    if (oneway) {
        p = p.ice_oneway();
    }

    await p.ice_ping();
    const connection = await p.ice_getConnection();
    test(connection !== null);

    // The inactivity timeout is 3s on the client side and 5s on the server side; 4 seconds tests only the
    // client-side.
    await p.sleep(4000); // two-way blocks for 4 seconds; one-way is non-blocking
    if (oneway) {
        await Ice.Promise.delay(4000);
    }
    await p.ice_ping();
    const connection2 = await p.ice_getConnection();

    if (oneway) {
        // With a oneway invocation, the inactivity timeout on the client side shut down the first connection.
        test(connection2 != connection);
    } else {
        // With a two-way invocation, the inactivity timeout should not shutdown any connection.
        test(connection2 == connection);
    }
    output.writeLine("ok");
}

export class Client extends TestHelper {
    async allTests(): Promise<void> {
        const communicator = this.communicator();

        const p = new Test.TestIntfPrx(communicator, `test: ${this.getTestEndpoint()}`);

        await testClientInactivityTimeout(p, this);
        await testWithOutstandingRequest(p, false, this);
        await testWithOutstandingRequest(p, true, this);

        await p.shutdown();
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            // We configure a low idle timeout to make sure we send heartbeats frequently. It's the sending of the
            // heartbeats that schedules the inactivity timer task.
            properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
            properties.setProperty("Ice.Connection.Client.InactivityTimeout", "3");
            [communicator] = this.initialize(properties);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
