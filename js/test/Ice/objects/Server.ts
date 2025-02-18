// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test as Test_Test } from "./Test.js";
import { Test as Test_Forward } from "./Forward.js";

const Test = {
    ...Test_Test,
    ...Test_Forward,
};
Test.Inner = Test_Test.Inner;

import { TestHelper } from "../../Common/TestHelper.js";
import { InitialI } from "./InitialI.js";

class UnexpectedObjectExceptionTestI extends Test.UnexpectedObjectExceptionTest {
    op(current: Ice.Current) {
        return new Test.AlsoEmpty();
    }
}

class F2I extends Test.F2 {
    op(current: Ice.Current) {}
}

export class Server extends TestHelper {
    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        let echo: Test_Test.EchoPrx | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Warn.Dispatch", "0");
            properties.setProperty("Ice.Warn.Connections", "0");
            [communicator] = this.initialize(properties);
            echo = new Test.EchoPrx(communicator, `__echo:${this.getTestEndpoint()}`);
            const adapter = await communicator.createObjectAdapter("");
            adapter.add(new InitialI(communicator), Ice.stringToIdentity("initial"));
            adapter.add(new F2I(), Ice.stringToIdentity("F21"));
            adapter.add(new UnexpectedObjectExceptionTestI(), Ice.stringToIdentity("uoet"));
            await echo.setConnection();
            echo.ice_getCachedConnection().setAdapter(adapter);
            this.serverReady();
            await communicator.waitForShutdown();
        } finally {
            if (echo) {
                await echo.shutdown();
            }

            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
