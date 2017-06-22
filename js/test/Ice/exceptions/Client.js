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

    var allTests = function(out, communicator, Test)
    {
        class EmptyI extends Test.Empty
        {
        }

        class ServantLocatorI
        {
            locate(curr, cookie)
            {
                return null;
            }

            finished(curr, servant, cookie)
            {
            }

            deactivate(category)
            {
            }
        }

        function ValueFactoryI()
        {
            return null;
        }

        var p = new Ice.Promise();
        var test = function(b, ex)
        {
            if(!b)
            {
                try
                {
                    var msg = "test failed";
                    if(ex)
                    {
                        msg += ": " + ex.toString();
                    }
                    throw new Error(msg);
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        var failCB = function(){ test(false); };

        var supportsUndeclaredExceptions = function(thrower)
        {
            return Ice.Promise.try(
                function()
                {
                    return thrower.supportsUndeclaredExceptions().then(
                        function(v)
                        {
                            if(v)
                            {
                                out.write("catching unknown user exception... ");
                                return thrower.throwUndeclaredA(1).then(
                                    failCB,
                                    function(ex)
                                    {
                                        test(ex instanceof Ice.UnknownUserException, ex);
                                        return thrower.throwUndeclaredB(1, 2);
                                    }
                                ).then(
                                    failCB,
                                    function(ex)
                                    {
                                        test(ex instanceof Ice.UnknownUserException, ex);
                                        return thrower.throwUndeclaredC(1, 2, 3);
                                    }
                                ).then(
                                    failCB,
                                    function(ex)
                                    {
                                        test(ex instanceof Ice.UnknownUserException, ex);
                                        out.writeLine("ok");
                                    }
                                );
                            }
                        });
                });
        };

        var supportsAssertException = function(thrower)
        {
            return Ice.Promise.try(
                function()
                {
                    return thrower.supportsAssertException().then(
                        function(v)
                        {
                            if(v)
                            {
                                out.write("testing assert in the server... ");
                                return thrower.throwAssertException().then(
                                    failCB,
                                    function(ex)
                                    {
                                        test(ex instanceof Ice.ConnectionLostException ||
                                             ex instanceof Ice.UnknownException, ex);
                                        out.writeLine("ok");
                                    }
                                );
                            }
                        });
                });
        };

        var base, ref, thrower;
        Ice.Promise.try(
            function()
            {
                out.write("testing object adapter registration exceptions... ");
                return communicator.createObjectAdapter("TestAdapter0").then(
                    failCB,
                    function(ex)
                    {
                        test(ex instanceof Ice.InitializationException, ex); // Expected
                    });
            }
        ).then(
            function()
            {
                return communicator.createObjectAdapterWithEndpoints("TestAdapter0", "default").then(
                    failCB,
                    function(ex)
                    {
                        test(ex instanceof Ice.FeatureNotSupportedException, ex); // Expected
                        out.writeLine("ok");
                    });
            }
        ).then(
            function()
            {
                out.write("testing servant registration exceptions... ");
                return communicator.createObjectAdapter("").then(
                    function(adapter)
                    {
                        var obj = new EmptyI();
                        adapter.add(obj, Ice.stringToIdentity("x"));
                        try
                        {
                            adapter.add(obj, Ice.stringToIdentity("x"));
                            test(false);
                        }
                        catch(ex)
                        {
                            test(ex instanceof Ice.AlreadyRegisteredException, ex);
                        }
                        try
                        {
                            adapter.add(obj, Ice.stringToIdentity(""));
                            test(false);
                        }
                        catch(ex)
                        {
                            test(ex instanceof Ice.IllegalIdentityException, ex);
                            test(ex.id.name === "");
                        }
                        try
                        {
                            adapter.add(null, Ice.stringToIdentity("x"));
                            test(false);
                        }
                        catch(ex)
                        {
                            test(ex instanceof Ice.IllegalServantException, ex);
                        }

                        adapter.remove(Ice.stringToIdentity("x"));
                        try
                        {
                            adapter.remove(Ice.stringToIdentity("x"));
                            test(false);
                        }
                        catch(ex)
                        {
                            test(ex instanceof Ice.NotRegisteredException, ex);
                        }
                        adapter.deactivate();
                        out.writeLine("ok");
                    });
            }
        ).then(
            function()
            {
                out.write("testing servant locator registration exceptions... ");
                return communicator.createObjectAdapter("").then(
                    function(adapter)
                    {
                        var loc = new ServantLocatorI();
                        adapter.addServantLocator(loc, "x");
                        try
                        {
                            adapter.addServantLocator(loc, "x");
                            test(false);
                        }
                        catch(ex)
                        {
                            test(ex instanceof Ice.AlreadyRegisteredException, ex);
                        }
                        adapter.deactivate();
                        out.writeLine("ok");
                    });
            }
        ).then(
            function()
            {
                out.write("testing value factory registration exception... ");

                communicator.getValueFactoryManager().add(ValueFactoryI, "::x");
                try
                {
                    communicator.getValueFactoryManager().add(ValueFactoryI, "::x");
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.AlreadyRegisteredException, ex);
                }
                out.writeLine("ok");

                out.write("testing stringToProxy... ");
                ref = "thrower:default -p 12010";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                out.writeLine("ok");
                out.write("testing checked cast... ");
                return Test.ThrowerPrx.checkedCast(base);
            }
        ).then(
            function(obj)
            {
                thrower = obj;
                test(thrower !== null);
                test(thrower.equals(base));
                out.writeLine("ok");
                out.write("catching exact types... ");
                return thrower.throwAasA(1);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.A, ex);
                test(ex.aMem === 1);
                return thrower.throwAorDasAorD(1);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.A, ex);
                test(ex.aMem === 1);
                return thrower.throwAorDasAorD(-1);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.D, ex);
                test(ex.dMem === -1);
                return thrower.throwBasB(1, 2);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.B, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                return thrower.throwCasC(1, 2, 3);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.C, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
                out.writeLine("ok");
                out.write("catching base types... ");
                return thrower.throwBasB(1, 2);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.A, ex);
                test(ex.aMem == 1);
                return thrower.throwCasC(1, 2, 3);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.B, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                out.writeLine("ok");
                out.write("catching derived types... ");
                return thrower.throwBasA(1, 2);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.B, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                return thrower.throwCasA(1, 2, 3);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.C, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
                return thrower.throwCasB(1, 2, 3);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.C, ex);
                test(ex.aMem == 1);
                test(ex.bMem == 2);
                test(ex.cMem == 3);
                out.writeLine("ok");
                return supportsUndeclaredExceptions(thrower);
            }
        ).then(
            function()
            {
                return supportsAssertException(thrower);
            }
        ).then(
            function()
            {
                out.write("testing memory limit marshal exception...");
                return thrower.throwMemoryLimitException(null).then(
                    failCB,
                    function(ex)
                    {
                        test(ex instanceof Ice.MemoryLimitException, ex);
                        return thrower.throwMemoryLimitException(new Uint8Array(20 * 1024));
                    }
                ).then(
                    failCB,
                    function(ex)
                    {
                        test(ex.toString().indexOf("ConnectionLostException") > 0, ex);
                        out.writeLine("ok");
                    }
                );
            }
        ).then(
            function()
            {
                out.write("catching object not exist exception... ");
                var id = Ice.stringToIdentity("does not exist");
                var thrower2 = Test.ThrowerPrx.uncheckedCast(thrower.ice_identity(id));
                return thrower2.ice_ping();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.ObjectNotExistException, ex);
                test(ex.id.equals(Ice.stringToIdentity("does not exist")));
                out.writeLine("ok");
                out.write("catching facet not exist exception... ");
                var thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet");
                return thrower2.ice_ping();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.FacetNotExistException, ex);
                test(ex.facet == "no such facet");
                out.writeLine("ok");
                out.write("catching operation not exist exception... ");
                var thrower2 = Test.WrongOperationPrx.uncheckedCast(thrower);
                return thrower2.noSuchOperation();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.OperationNotExistException, ex);
                test(ex.operation == "noSuchOperation");
                out.writeLine("ok");
                out.write("catching unknown local exception... ");
                return thrower.throwLocalException();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.UnknownLocalException, ex);
                return thrower.throwLocalExceptionIdempotent();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.UnknownLocalException ||
                     ex instanceof Ice.OperationNotExistException, ex);
                out.writeLine("ok");
                out.write("catching unknown non-Ice exception... ");
                return thrower.throwNonIceException();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.UnknownException, ex);
                out.writeLine("ok");
                out.write("testing asynchronous exceptions... ");
                return thrower.throwAfterResponse();
            }
        ).then(
            function()
            {
                return thrower.throwAfterException();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.A);
                out.writeLine("ok");
                return thrower.shutdown();
            }
        ).then(p.resolve, p.reject);
        return p;
    };

    var run = function(out, id)
    {
        id.properties.setProperty("Ice.MessageSizeMax", "10");
        id.properties.setProperty("Ice.Warn.Connections", "0");
        id.properties.setProperty("Ice.PrintStackTraces", "1");
        var c = Ice.initialize(id);
        return Ice.Promise.try(
            function()
            {
                return allTests(out, c, Test);
            }
        ).finally(() => c.destroy);
    };
    exports._test = run;
    exports._clientAllTests = allTests;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
