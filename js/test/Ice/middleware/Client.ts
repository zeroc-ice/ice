// Copyright (c) ZeroC, Inc.

import { Ice } from "ice";
import { TestHelper } from "../../Common/TestHelper.js";
import { Test } from "./Test.js";

export class Client extends TestHelper {
    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            const [properties] = this.createTestProperties(args);
            [communicator] = this.initialize(properties);

            let prx = new Test.MyObjectPrx(communicator, `test: ${this.getTestEndpoint()}`);
            await prx.getName();
            await prx.shutdown();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
