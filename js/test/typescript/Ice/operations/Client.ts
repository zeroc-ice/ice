// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./Test"
import {TestHelper} from "../../../Common/TestHelper"
import * as Twoways from "./Twoways";
import * as Oneways from "./Oneways";
import * as BatchOneways from "./BatchOneways";

export class Client extends TestHelper
{
    async allTests(bidir:boolean)
    {
        const out = this.getWriter();
        const communicator = this.communicator();
        out.write("testing twoway operations... ");
        const ref = "test:" + this.getTestEndpoint();
        const base = communicator.stringToProxy(ref);
        const cl = await Test.MyClassPrx.checkedCast(base);
        const derived = await Test.MyDerivedClassPrx.checkedCast(cl);

        await Twoways.run(communicator, cl, bidir, this);
        await Twoways.run(communicator, derived, bidir, this);
        out.writeLine("ok");

        out.write("testing oneway operations... ");
        await Oneways.run(communicator, cl, bidir);
        out.writeLine("ok");

        out.write("testing batch oneway operations... ");
        await BatchOneways.run(communicator, cl, bidir);
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

    async run(args:string[])
    {
        let communicator:Ice.Communicator;
        try
        {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.BatchAutoFlushSize", "100");
            [communicator] = this.initialize(properties);
            await this.allTests(false);
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
