// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";
import { AMDThrowerI } from "./AMDThrowerI.js";

export class ServerAMD extends TestHelper {
    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        let echo: Test.EchoPrx | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.MessageSizeMax", "10");
            properties.setProperty("Ice.Warn.Dispatch", "0");
            properties.setProperty("Ice.Warn.Connections", "0");
            [communicator] = this.initialize(properties);
            echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));
            const adapter = await communicator.createObjectAdapter("");
            adapter.add(new AMDThrowerI(), Ice.stringToIdentity("thrower"));
            await echo.setConnection();
            const connection = echo.ice_getCachedConnection();
            connection.setCloseCallback((con) => {
                // Re-establish connection if it fails (necessary for MemoryLimitException test)
                echo!.setConnection().then(() => echo!.ice_getCachedConnection().setAdapter(adapter));
            });
            connection.setAdapter(adapter);
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
