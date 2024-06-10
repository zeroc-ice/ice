//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice } from "ice";
import { Test as Test_Test } from "./Test.js";
import { Test as Test_Forward } from "./Forward.js";

const Test = {
    ...Test_Test,
    ...Test_Forward
};
Test.Inner = Test_Test.Inner;

import { TestHelper } from "../../Common/TestHelper.js";
import { InitialI } from "./InitialI.js";

class UnexpectedObjectExceptionTestI extends Test.UnexpectedObjectExceptionTest
{
    op(current)
    {
        return new Test.AlsoEmpty();
    }
}

class F2I extends Test.F2
{
    op(current)
    {
    }
}

export class Server extends TestHelper
{
    async run(args)
    {
        let communicator;
        let echo;
        try
        {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Warn.Dispatch", "0");
            properties.setProperty("Ice.Warn.Connections", "0");
            [communicator] = this.initialize(properties);
            echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));
            const adapter = await communicator.createObjectAdapter("");
            adapter.add(new InitialI(communicator), Ice.stringToIdentity("initial"));
            adapter.add(new F2I(), Ice.stringToIdentity("F21"));
            adapter.add(new UnexpectedObjectExceptionTestI(), Ice.stringToIdentity("uoet"));
            await echo.setConnection();
            echo.ice_getCachedConnection().setAdapter(adapter);
            this.serverReady();
            await communicator.waitForShutdown();
        }
        finally
        {
            if(echo)
            {
                await echo.shutdown();
            }

            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }
}
