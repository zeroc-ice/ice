//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice } from "ice";
import { Test } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

import { twoways } from "./Twoways.js";
import { oneways } from "./Oneways.js";
import { batchOneways } from "./BatchOneways.js";

export class Client extends TestHelper
{
    async allTests(Test, bidir)
    {
        const out = this.getWriter();
        const communicator = this.communicator();
        out.write("testing twoway operations... ");
        const ref = "test:" + this.getTestEndpoint();
        const base = communicator.stringToProxy(ref);
        const cl = await Test.MyClassPrx.checkedCast(base);
        const derived = await Test.MyDerivedClassPrx.checkedCast(cl);

        await twoways(communicator, cl, Test, bidir, this);
        await twoways(communicator, derived, Test, bidir, this);
        out.writeLine("ok");

        out.write("testing oneway operations... ");
        await oneways(communicator, cl, Test, bidir);
        out.writeLine("ok");

        out.write("testing batch oneway operations... ");
        await batchOneways(communicator, cl, Test, bidir);
        out.writeLine("ok");

        out.write("testing server shutdown... ");
        await cl.shutdown();
        try
        {
            await cl.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
            throw new Error("test failed");
        }
        catch(ex)
        {
            if(ex instanceof Ice.LocalException)
            {
                out.writeLine("ok");
            }
            else
            {
                throw ex;
            }
        }
    }

    async run(args)
    {
        let communicator;
        try
        {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.BatchAutoFlushSize", "100");
            [communicator] = this.initialize(properties);
            await this.allTests(Test, false);
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }
}
