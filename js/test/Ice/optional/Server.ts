// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";
import { InitialI } from "./InitialI.js";

export class Server extends TestHelper {
    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        let echo: Test.EchoPrx | null = null;
        try {
            [communicator] = this.initialize(args);
            echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));
            const adapter = await communicator.createObjectAdapter("");
            adapter.add(new InitialI(), Ice.stringToIdentity("initial"));
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
