// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Glacier2 = require("ice").Glacier2;
    var Test = require("Callback").Test;
    var Promise = Ice.Promise;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var CallbackPrx = Test.CallbackPrx;
    var CallbackReceiverPrx = Test.CallbackReceiverPrx;

    class CallbackReceiverI extends Test.CallbackReceiver
    {
        constructor()
        {
            super();
            this._callback = false;
            this._p = new Promise();
        }


        callback(current)
        {
            test(!this._callback);
            this._p.resolve();
        }

        callbackEx(current)
        {
            this.callback(current);
            var ex = new Test.CallbackException();
            ex.someValue = 3.14;
            ex.someString = "3.14";
            throw ex;
        }

        callbackOK()
        {
            var p = new Promise();
            this._p.then(() =>
                {
                    p.resolve();
                    this._callback = false;
                    this._p = new Promise();
                });
            return p;
        }
    }

    var allTests = function(out, communicator)
    {
        var p = new Promise();

        var failCB = function () { test(false); };

        var router, base, session, twoway, oneway, category, processBase, processPrx, adapter,callbackReceiverImpl,
            callbackReceiver,
            twowayR, onewayR,
            fakeTwowayR;

        return Promise.try(
            function()
            {
                out.write("testing stringToProxy for router... ");
                var routerBase = communicator.stringToProxy("Glacier2/router:default -p 12020");
                test(routerBase !== null);
                out.writeLine("ok");

                out.write("testing checked cast for router... ");
                return Glacier2.RouterPrx.checkedCast(routerBase);
            }
        ).then(
            function(o)
            {
                router = o;
                test(router !== null);
                out.writeLine("ok");

                out.write("installing router with communicator... ");
                communicator.setDefaultRouter(router);
                out.writeLine("ok");

                out.write("getting the session timeout... ");
                return router.getSessionTimeout();
            }
        ).then(
            function(timeout)
            {
                test(timeout.low === 30);
                out.writeLine("ok");

                out.write("testing stringToProxy for server object... ");
                base = communicator.stringToProxy("c1/callback:default -p 12010");
                out.writeLine("ok");

                out.write("trying to ping server before session creation... ");
                return base.ice_ping();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.ConnectionLostException);
                out.writeLine("ok");
                out.write("trying to create session with wrong password... ");
                return router.createSession("userid", "xxx");
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Glacier2.PermissionDeniedException);
                out.writeLine("ok");

                out.write("trying to destroy non-existing session... ");
                return router.destroySession();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Glacier2.SessionNotExistException);
                out.writeLine("ok");

                out.write("creating session with correct password... ");
                return router.createSession("userid", "abc123");
            }
        ).then(
            function(s)
            {
                session = s;
                out.writeLine("ok");

                out.write("trying to create a second session... ");
                return router.createSession("userid", "abc123");
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Glacier2.CannotCreateSessionException);
                out.writeLine("ok");

                out.write("pinging server after session creation... ");
                return base.ice_ping();
            }
        ).then(
            function()
            {
                out.writeLine("ok");

                out.write("testing checked cast for server object... ");
                return Test.CallbackPrx.checkedCast(base);
            }
        ).then(
            function(o)
            {
                twoway = o;
                test(twoway !== null);
                out.writeLine("ok");
                out.write("creating and activating callback receiver adapter... ");
                communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");
                return communicator.createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
            }
        ).then(
            function(o)
            {
                adapter = o;
                return adapter.activate();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("getting category from router... ");
                return router.getCategoryForClient();
            }
        ).then(
            function(c)
            {
                category = c;
                out.writeLine("ok");
                out.write("creating and adding callback receiver object... ");
                callbackReceiverImpl = new CallbackReceiverI();
                callbackReceiver = callbackReceiverImpl;
                var callbackReceiverIdent = new Ice.Identity();
                callbackReceiverIdent.name = "callbackReceiver";
                callbackReceiverIdent.category = category;
                twowayR = CallbackReceiverPrx.uncheckedCast(adapter.add(callbackReceiver, callbackReceiverIdent));
                var fakeCallbackReceiverIdent = new Ice.Identity();
                fakeCallbackReceiverIdent.name = "callbackReceiver";
                fakeCallbackReceiverIdent.category = "dummy";
                fakeTwowayR = CallbackReceiverPrx.uncheckedCast(
                    adapter.add(callbackReceiver, fakeCallbackReceiverIdent));
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing oneway callback... ");
                oneway = CallbackPrx.uncheckedCast(twoway.ice_oneway());
                onewayR = CallbackReceiverPrx.uncheckedCast(twowayR.ice_oneway());
                var context = new Ice.Context();
                context.set("_fwd", "o");
                return oneway.initiateCallback(onewayR, context);
            }
        ).then(
            function()
            {
                return callbackReceiverImpl.callbackOK();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing twoway callback... ");
                var context = new Ice.Context();
                context.set("_fwd", "t");
                return twoway.initiateCallback(twowayR, context);
            }
        ).then(
            function()
            {
                return callbackReceiverImpl.callbackOK();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("ditto, but with user exception... ");
                var context = new Ice.Context();
                context.set("_fwd", "t");
                return twoway.initiateCallbackEx(twowayR, context);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Test.CallbackException);
                test(ex.someValue == 3.14);
                test(ex.someString == "3.14");
                return callbackReceiverImpl.callbackOK();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("trying twoway callback with fake category... ");
                var context = new Ice.Context();
                context.set("_fwd", "t");
                return twoway.initiateCallback(fakeTwowayR, context);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.ObjectNotExistException);
                out.writeLine("ok");
                out.write("testing whether other allowed category is accepted... ");
                var context = new Ice.Context();
                context.set("_fwd", "t");
                var otherCategoryTwoway = CallbackPrx.uncheckedCast(
                    twoway.ice_identity(Ice.stringToIdentity("c2/callback")));
                return otherCategoryTwoway.initiateCallback(twowayR, context);
            }
        ).then(
            function()
            {
                return callbackReceiverImpl.callbackOK();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing whether disallowed category gets rejected... ");
                var context = new Ice.Context();
                context.set("_fwd", "t");
                var otherCategoryTwoway = CallbackPrx.uncheckedCast(
                    twoway.ice_identity(Ice.stringToIdentity("c3/callback")));
                return otherCategoryTwoway.initiateCallback(twowayR, context);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.ObjectNotExistException);
                out.writeLine("ok");
                out.write("testing whether user-id as category is accepted... ");
                var context = new Ice.Context();
                context.set("_fwd", "t");
                var otherCategoryTwoway = CallbackPrx.uncheckedCast(
                    twoway.ice_identity(Ice.stringToIdentity("_userid/callback")));
                return otherCategoryTwoway.initiateCallback(twowayR, context);
            }
        ).then(
            function()
            {
                return callbackReceiverImpl.callbackOK();
            }
        ).then(
            function()
            {
                if(process.argv.indexOf("--shutdown") > -1)
                {
                    out.writeLine("ok");
                    out.write("testing server shutdown... ");
                    return twoway.shutdown();
                    // No ping, otherwise the router prints a warning message if it's
                    // started with --Ice.Warn.Connections.
                }
            }
        ).then(
            function()
            {
                out.writeLine("ok");

                out.write("destroying session... ");
                return router.destroySession();
            }
        ).then(
            function()
            {
                out.writeLine("ok");

                out.write("trying to ping server after session destruction... ");
                return base.ice_ping();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.ConnectionLostException);
                out.writeLine("ok");

                if(process.argv.indexOf("--shutdown") > -1)
                {
                    out.write("uninstalling router with communicator... ");
                    communicator.setDefaultRouter(null);
                    out.writeLine("ok");

                    out.write("testing stringToProxy for process object... ");
                    processBase = communicator.stringToProxy("Glacier2/admin -f Process:default -h 127.0.0.1 -p 12021");
                    out.writeLine("ok");

                    out.write("testing checked cast for admin object... ");
                    return Ice.ProcessPrx.checkedCast(processBase).then(
                        function(o)
                        {
                            processPrx = o;
                            test(processPrx !== null);
                            out.writeLine("ok");

                            out.write("testing Glacier2 shutdown... ");
                            return processPrx.shutdown();
                        }
                    ).then(
                        function()
                        {
                            return processPrx.ice_ping();
                        }
                    ).then(
                        failCB,
                        function(ex)
                        {
                            test(ex instanceof Ice.LocalException);
                            out.writeLine("ok");
                        }
                    );
                }
            }
        ).catch(e => console.log(e));
    };

    var run = function(out, id)
    {
        return Promise.try(
            function()
            {
                id.properties.setProperty("Ice.Warn.Dispatch", "1");
                id.properties.setProperty("Ice.Warn.Connections", "0");
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
