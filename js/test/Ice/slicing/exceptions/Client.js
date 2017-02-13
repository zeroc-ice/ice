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
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;
    var Promise = Ice.Promise;
    var ArrayUtil = Ice.ArrayUtil;

    var allTests = function(out, communicator)
    {
        var failCB = function(){ test(false); };
        var p = new Promise();
        var test = function(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        var ref, base, prx;

        Promise.try(() =>
            {
                out.write("testing stringToProxy... ");
                ref = "Test:default -p 12010 -t 10000";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                out.writeLine("ok");

                out.write("testing checked cast... ");
                return Test.TestIntfPrx.checkedCast(base);
            }
        ).then(obj =>
            {
                prx = obj;
                test(prx !== null);
                test(prx.equals(base));
                out.writeLine("ok");
                out.write("base... ");
                return prx.baseAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.Base.prototype);
                test(ex.b == "Base.b");
                test(ex.ice_name() == "Test::Base");

                out.writeLine("ok");
                out.write("slicing of unknown derived... ");
                return prx.unknownDerivedAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.Base.prototype);
                test(ex.b == "UnknownDerived.b");
                test(ex.ice_name() == "Test::Base");

                out.writeLine("ok");
                out.write("non-slicing of known derived as base... ");
                return prx.knownDerivedAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownDerived.prototype);
                test(ex.b == "KnownDerived.b");
                test(ex.kd == "KnownDerived.kd");
                test(ex.ice_name() == "Test::KnownDerived");
                out.writeLine("ok");
                out.write("non-slicing of known derived as derived... ");

                return prx.knownDerivedAsKnownDerived();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownDerived.prototype);
                test(ex.b == "KnownDerived.b");
                test(ex.kd == "KnownDerived.kd");
                test(ex.ice_name() == "Test::KnownDerived");

                out.writeLine("ok");
                out.write("slicing of unknown intermediate as base... ");
                return prx.unknownIntermediateAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.Base.prototype);
                test(ex.b == "UnknownIntermediate.b");
                test(ex.ice_name() == "Test::Base");

                out.writeLine("ok");
                out.write("slicing of known intermediate as base... ");
                return prx.knownIntermediateAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownIntermediate.prototype);
                test(ex.b == "KnownIntermediate.b");
                test(ex.ki == "KnownIntermediate.ki");
                test(ex.ice_name() == "Test::KnownIntermediate");

                out.writeLine("ok");
                out.write("slicing of known most derived as base... ");
                return prx.knownMostDerivedAsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownMostDerived.prototype);
                test(ex.b == "KnownMostDerived.b");
                test(ex.ki == "KnownMostDerived.ki");
                test(ex.kmd == "KnownMostDerived.kmd");
                test(ex.ice_name() == "Test::KnownMostDerived");

                out.writeLine("ok");
                out.write("non-slicing of known intermediate as intermediate... ");

                return prx.knownIntermediateAsKnownIntermediate();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownIntermediate.prototype);
                test(ex.b == "KnownIntermediate.b");
                test(ex.ki == "KnownIntermediate.ki");
                test(ex.ice_name() == "Test::KnownIntermediate");
                out.writeLine("ok");
                out.write("non-slicing of known most derived as intermediate... ");
                return prx.knownMostDerivedAsKnownIntermediate();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownMostDerived.prototype);
                test(ex.b == "KnownMostDerived.b");
                test(ex.ki == "KnownMostDerived.ki");
                test(ex.kmd == "KnownMostDerived.kmd");
                test(ex.ice_name() == "Test::KnownMostDerived");
                out.writeLine("ok");
                out.write("non-slicing of known most derived as most derived... ");

                return prx.knownMostDerivedAsKnownMostDerived();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownMostDerived.prototype);
                test(ex.b == "KnownMostDerived.b");
                test(ex.ki == "KnownMostDerived.ki");
                test(ex.kmd == "KnownMostDerived.kmd");
                test(ex.ice_name() == "Test::KnownMostDerived");

                out.writeLine("ok");
                out.write("slicing of unknown most derived, known intermediate as base... ");
                return prx.unknownMostDerived1AsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownIntermediate.prototype);
                test(ex.b == "UnknownMostDerived1.b");
                test(ex.ki == "UnknownMostDerived1.ki");
                test(ex.ice_name() == "Test::KnownIntermediate");
                out.writeLine("ok");
                out.write("slicing of unknown most derived, known intermediate as intermediate... ");
                return prx.unknownMostDerived1AsKnownIntermediate();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.KnownIntermediate.prototype);
                test(ex.b == "UnknownMostDerived1.b");
                test(ex.ki == "UnknownMostDerived1.ki");
                test(ex.ice_name() == "Test::KnownIntermediate");

                out.writeLine("ok");
                out.write("slicing of unknown most derived, unknown intermediate thrown as base... ");

                return prx.unknownMostDerived2AsBase();
            }
        ).then(
            failCB,
            ex =>
            {
                test(Object.getPrototypeOf(ex) === Test.Base.prototype);
                test(ex.b == "UnknownMostDerived2.b");
                test(ex.ice_name() == "Test::Base");

                out.writeLine("ok");
                out.write("unknown most derived in compact format... ");

                return prx.unknownMostDerived2AsBaseCompact();
            }
        ).then(
            failCB,
            ex =>
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
                out.writeLine("ok");
                return prx.shutdown();
            }
        ).then(p.resolve, p.reject);
        return p;
    };

    var run = function(out, id)
    {
        return Promise.try(
            function()
            {
                var c = Ice.initialize(id);
                return allTests(out, c).finally(
                    function()
                    {
                        if(c)
                        {
                            return c.destroy();
                        }
                    });
            });
    };

    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
