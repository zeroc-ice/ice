// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";
import {InitialI} from "./InitialI";

const test = TestHelper.test;

class UnexpectedObjectExceptionTestI extends Test.UnexpectedObjectExceptionTest
{
    op(current:Ice.Current):Test.Empty
    {
        return new Test.AlsoEmpty();
    }
}

export class Server extends TestHelper
{
    async run(args:string[])
    {
        let communicator:Ice.Communicator;
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
