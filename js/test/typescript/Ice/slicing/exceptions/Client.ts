//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../../Common/TestHelper";

const test = TestHelper.test;

export class Client extends TestHelper
{
    async allTests()
    {
        const out = this.getWriter();
        const communicator = this.communicator();

        out.write("testing stringToProxy... ");
        const ref = "Test:" + this.getTestEndpoint() + " -t 10000";
        const base = communicator.stringToProxy(ref);
        test(base !== null);
        out.writeLine("ok");

        out.write("testing checked cast... ");
        const prx = await Test.TestIntfPrx.checkedCast(base);
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
            test(ex instanceof Test.Base, ex);
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
            test(ex instanceof Test.Base, ex);
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
            test(ex instanceof Test.KnownDerived, ex);
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
            test(ex instanceof Test.KnownDerived, ex);
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
            test(ex instanceof Test.Base, ex);
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
            test(ex instanceof Test.KnownIntermediate, ex);
            test(ex.b == "KnownIntermediate.b");
            test(ex.ki == "KnownIntermediate.ki");
            test(ex.ice_id() == "::Test::KnownIntermediate");
        }
        out.writeLine("ok");

        out.write("slicing of known most derived as base... ");
        try
        {
            await prx.knownMostDerivedAsBase();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.KnownMostDerived, ex);
            test(ex.b == "KnownMostDerived.b");
            test(ex.ki == "KnownMostDerived.ki");
            test(ex.kmd == "KnownMostDerived.kmd");
            test(ex.ice_id() == "::Test::KnownMostDerived");
        }
        out.writeLine("ok");

        out.write("non-slicing of known intermediate as intermediate... ");
        try
        {
            await prx.knownIntermediateAsKnownIntermediate();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.KnownIntermediate, ex);
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
            test(ex instanceof Test.KnownMostDerived, ex);
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
            test(ex instanceof Test.KnownMostDerived, ex);
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
            test(ex instanceof Test.KnownIntermediate, ex);
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
            test(ex instanceof Test.KnownIntermediate, ex);
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
            test(ex instanceof Test.Base, ex);
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
            if(ex instanceof Test.Base)
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to Base.
                //
                test(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0));
            }
            else if(ex instanceof Ice.UnknownUserException)
            {
                //
                // An UnknownUserException is raised for the compact format because the
                // most-derived type is unknown and the exception cannot be sliced.
                //
                test(!prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0));
            }
            else if(ex instanceof Ice.OperationNotExistException)
            {
                // Ignore
            }
            else
            {
                test(false, ex);
            }
        }
        out.writeLine("ok");

        out.write("preserved exceptions...");
        try
        {
            await prx.unknownPreservedAsBase();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.Base, ex);
            if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
            {
                test(ex.ice_getSlicedData() === null);
            }
            else
            {
                const slicedData = ex.ice_getSlicedData();
                test(slicedData !== null);
                test(slicedData.slices.length == 2);
                test(slicedData.slices[1].typeId == "::Test::SPreserved1");
                test(slicedData.slices[0].typeId == "::Test::SPreserved2");
            }
        }

        try
        {
            await prx.unknownPreservedAsKnownPreserved();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.KnownPreserved, ex);
            test(ex.kp == "preserved");
            if(prx.ice_getEncodingVersion().equals(Ice.Encoding_1_0))
            {
                test(ex.ice_getSlicedData() === null);
            }
            else
            {
                const slicedData = ex.ice_getSlicedData();
                test(slicedData !== null);
                test(slicedData.slices.length == 2);
                test(slicedData.slices[1].typeId == "::Test::SPreserved1");
                test(slicedData.slices[0].typeId == "::Test::SPreserved2");
            }
        }
        out.writeLine("ok");
        await prx.shutdown();
    }

    async run(args:string[])
    {
        let communicator:Ice.Communicator;
        try
        {
            [communicator] = this.initialize(args);
            await this.allTests();
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
