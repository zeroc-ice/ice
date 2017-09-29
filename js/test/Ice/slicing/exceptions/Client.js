// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const ArrayUtil = Ice.ArrayUtil;

    async function allTests(out, communicator)
    {
        function test(value)
        {
            if(!value)
            {
                throw new Error("test failed");
            }
        }

        out.write("testing stringToProxy... ");
        let ref = "Test:default -p 12010 -t 10000";
        let base = communicator.stringToProxy(ref);
        test(base !== null);
        out.writeLine("ok");

        out.write("testing checked cast... ");
        let prx = await Test.TestIntfPrx.checkedCast(base);
        test(prx !== null);
        test(prx.equals(base));
        out.writeLine("ok");

        out.write("base... ");
        try
        {
            await prx.baseAsBase();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.Base.prototype);
            test(ex.b == "Base.b");
            test(ex.ice_id() == "::Test::Base");
        }
        out.writeLine("ok");

        out.write("slicing of unknown derived... ");
        try
        {
            await prx.unknownDerivedAsBase();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.Base.prototype);
            test(ex.b == "UnknownDerived.b");
            test(ex.ice_id() == "::Test::Base");
        }
        out.writeLine("ok");

        out.write("non-slicing of known derived as base... ");
        try
        {
            await prx.knownDerivedAsBase();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownDerived.prototype);
            test(ex.b == "KnownDerived.b");
            test(ex.kd == "KnownDerived.kd");
            test(ex.ice_id() == "::Test::KnownDerived");
        }
        out.writeLine("ok");

        out.write("non-slicing of known derived as derived... ");
        try
        {
            await prx.knownDerivedAsKnownDerived();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownDerived.prototype);
            test(ex.b == "KnownDerived.b");
            test(ex.kd == "KnownDerived.kd");
            test(ex.ice_id() == "::Test::KnownDerived");
        }
        out.writeLine("ok");

        out.write("slicing of unknown intermediate as base... ");
        try
        {
            await prx.unknownIntermediateAsBase();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.Base.prototype);
            test(ex.b == "UnknownIntermediate.b");
            test(ex.ice_id() == "::Test::Base");
        }
        out.writeLine("ok");

        out.write("slicing of known intermediate as base... ");
        try
        {
            await prx.knownIntermediateAsBase();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownIntermediate.prototype);
            test(ex.b == "KnownIntermediate.b");
            test(ex.ki == "KnownIntermediate.ki");
            test(ex.ice_id() == "::Test::KnownIntermediate");
        }
        out.writeLine("ok");

        out.write("slicing of known most derived as base... ");
        try
        {
            await  prx.knownMostDerivedAsBase();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownMostDerived.prototype);
            test(ex.b == "KnownMostDerived.b");
            test(ex.ki == "KnownMostDerived.ki");
            test(ex.kmd == "KnownMostDerived.kmd");
            test(ex.ice_id() == "::Test::KnownMostDerived");
        }
        out.writeLine("ok");

        out.write("non-slicing of known intermediate as intermediate... ");
        try
        {
            await  prx.knownIntermediateAsKnownIntermediate();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownIntermediate.prototype);
            test(ex.b == "KnownIntermediate.b");
            test(ex.ki == "KnownIntermediate.ki");
            test(ex.ice_id() == "::Test::KnownIntermediate");
        }
        out.writeLine("ok");

        out.write("non-slicing of known most derived as intermediate... ");
        try
        {
            await prx.knownMostDerivedAsKnownIntermediate();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownMostDerived.prototype);
            test(ex.b == "KnownMostDerived.b");
            test(ex.ki == "KnownMostDerived.ki");
            test(ex.kmd == "KnownMostDerived.kmd");
            test(ex.ice_id() == "::Test::KnownMostDerived");
        }
        out.writeLine("ok");

        out.write("non-slicing of known most derived as most derived... ");
        try
        {
            await prx.knownMostDerivedAsKnownMostDerived();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownMostDerived.prototype);
            test(ex.b == "KnownMostDerived.b");
            test(ex.ki == "KnownMostDerived.ki");
            test(ex.kmd == "KnownMostDerived.kmd");
            test(ex.ice_id() == "::Test::KnownMostDerived");
        }
        out.writeLine("ok");

        out.write("slicing of unknown most derived, known intermediate as base... ");
        try
        {
            await prx.unknownMostDerived1AsBase();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownIntermediate.prototype);
            test(ex.b == "UnknownMostDerived1.b");
            test(ex.ki == "UnknownMostDerived1.ki");
            test(ex.ice_id() == "::Test::KnownIntermediate");
        }
        out.writeLine("ok");

        out.write("slicing of unknown most derived, known intermediate as intermediate... ");
        try
        {
            await prx.unknownMostDerived1AsKnownIntermediate();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.KnownIntermediate.prototype);
            test(ex.b == "UnknownMostDerived1.b");
            test(ex.ki == "UnknownMostDerived1.ki");
            test(ex.ice_id() == "::Test::KnownIntermediate");
        }
        out.writeLine("ok");

        out.write("slicing of unknown most derived, unknown intermediate thrown as base... ");
        try
        {
            await prx.unknownMostDerived2AsBase();
            test(false);
        }
        catch(ex)
        {
            test(Object.getPrototypeOf(ex) === Test.Base.prototype);
            test(ex.b == "UnknownMostDerived2.b");
            test(ex.ice_id() == "::Test::Base");
        }
        out.writeLine("ok");

        out.write("unknown most derived in compact format... ");
        try
        {
            await prx.unknownMostDerived2AsBaseCompact();
            test(false);
        }
        catch(ex)
        {
            if(Object.getPrototypeOf(ex) === Test.Base.prototype)
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to Base.
                //
                test(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0));
            }
            else if(Object.getPrototypeOf(ex) === Ice.UnknownUserException.prototype)
            {
                //
                // An UnknownUserException is raised for the compact format because the
                // most-derived type is unknown and the exception cannot be sliced.
                //
                test(!prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0));
            }
            else
            {
                test(false);
            }
        }
        out.writeLine("ok");

        out.write("preserved exceptions...")
        try
        {
            await prx.unknownPreservedAsBase();
        }
        catch(ex)
        {
            if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
            {
                test(ex.ice_getSlicedData() === null);
            }
            else
            {
                let slicedData = ex.ice_getSlicedData();
                test(slicedData !== null);
                test(slicedData.slices.length == 2);
                test(slicedData.slices[1].typeId == "::Test::SPreserved1");
                test(slicedData.slices[0].typeId == "::Test::SPreserved2");
            }
        }

        try
        {
            await prx.unknownPreservedAsKnownPreserved();
            test(false)
        }
        catch(ex)
        {
            test(ex.kp == "preserved");
            if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
            {
                test(ex.ice_getSlicedData() === null);
            }
            else
            {
                let slicedData = ex.ice_getSlicedData();
                test(slicedData !== null);
                test(slicedData.slices.length == 2);
                test(slicedData.slices[1].typeId == "::Test::SPreserved1");
                test(slicedData.slices[0].typeId == "::Test::SPreserved2");
            }
        }
        out.writeLine("ok");
        await prx.shutdown();
    }

    async function run(out, initData)
    {
        let communicator;
        try
        {
            communicator = Ice.initialize(initData);
            await allTests(out, communicator);
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
