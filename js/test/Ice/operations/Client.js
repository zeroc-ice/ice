// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    const Twoways = require("Twoways").Twoways;
    const Oneways = require("Oneways").Oneways;
    const BatchOneways = require("BatchOneways").BatchOneways;

    async function allTests(out, communicator, Test, bidir)
    {
        out.write("testing twoway operations... ");
        let ref = "test:default -p 12010";
        let base = communicator.stringToProxy(ref);
        let cl = await Test.MyClassPrx.checkedCast(base);
        let derived = await Test.MyDerivedClassPrx.checkedCast(cl);

        await Twoways.run(communicator, cl, Test, bidir);
        await Twoways.run(communicator, derived, Test, bidir)
        out.writeLine("ok");

        out.write("testing oneway operations... ");
        await Oneways.run(communicator, cl, Test, bidir);
        out.writeLine("ok");

        out.write("testing batch oneway operations... ");
        await BatchOneways.run(communicator, cl, Test, bidir);
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

    async function run(out, initData)
    {
        let communicator;
        try
        {
            initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
            communicator = Ice.initialize(initData);
            await allTests(out, communicator, Test, false);
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }

    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
