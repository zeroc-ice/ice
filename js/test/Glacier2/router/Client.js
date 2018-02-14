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
    const Glacier2 = require("ice").Glacier2;
    const Test = require("Callback").Test;

    function test(value, ex)
    {
        if(!value)
        {
            let message = "test failed";
            if(ex)
            {
                message += "\n" + ex.toString();
            }
            throw new Error(message);
        }
    }

    const CallbackPrx = Test.CallbackPrx;
    const CallbackReceiverPrx = Test.CallbackReceiverPrx;

    class CallbackReceiverI extends Test.CallbackReceiver
    {
        constructor()
        {
            super();
            this._callback = false;
            this._p = new Ice.Promise();
        }

        callback(current)
        {
            test(!this._callback);
            this._p.resolve();
        }

        callbackEx(current)
        {
            this.callback(current);
            throw new Test.CallbackException(3.14, "3.14");
        }

        callbackOK()
        {
            return this._p.then(() =>
                                {
                                    this._callback = false;
                                    this._p = new Ice.Promise();
                                });
        }
    }

    async function allTests(out, communicator)
    {
        out.write("testing stringToProxy for router... ");
        let routerBase = communicator.stringToProxy("Glacier2/router:default -p 12060");
        test(routerBase !== null);
        out.writeLine("ok");

        out.write("testing checked cast for router... ");
        let router = await Glacier2.RouterPrx.checkedCast(routerBase);
        test(router !== null);
        out.writeLine("ok");

        out.write("installing router with communicator... ");
        communicator.setDefaultRouter(router);
        out.writeLine("ok");

        out.write("getting the session timeout... ");
        let timeout = await router.getSessionTimeout();
        test(timeout.toNumber() === 30);
        out.writeLine("ok");

        out.write("testing stringToProxy for server object... ");
        base = communicator.stringToProxy("c1/callback:default -p 12010");
        out.writeLine("ok");

        out.write("trying to ping server before session creation... ");
        try
        {
            await base.ice_ping();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.ConnectionLostException, ex);
        }
        out.writeLine("ok");

        out.write("trying to create session with wrong password... ");
        try
        {
            await router.createSession("userid", "xxx");
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Glacier2.PermissionDeniedException, ex);
        }
        out.writeLine("ok");

        out.write("trying to destroy non-existing session... ");
        try
        {
            await router.destroySession();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Glacier2.SessionNotExistException, ex);
        }
        out.writeLine("ok");

        out.write("creating session with correct password... ");
        let session = await router.createSession("userid", "abc123");
        out.writeLine("ok");

        out.write("trying to create a second session... ");
        try
        {
            await router.createSession("userid", "abc123");
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Glacier2.CannotCreateSessionException, ex);
        }
        out.writeLine("ok");

        out.write("pinging server after session creation... ");
        await base.ice_ping();
        out.writeLine("ok");

        {
            out.write("pinging object with client endpoint... ");
            const baseC = communicator.stringToProxy("collocated:default -p 12060");
            try
            {
                await baseC.ice_ping();
            }
            catch(ex)
            {
            }
            out.writeLine("ok");
        }

        out.write("testing checked cast for server object... ");
        let twoway = await Test.CallbackPrx.checkedCast(base);
        test(twoway !== null);
        out.writeLine("ok");

        out.write("creating and activating callback receiver adapter... ");
        communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");
        let adapter = await communicator.createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
        await adapter.activate();
        out.writeLine("ok");

        out.write("getting category from router... ");
        let category = await router.getCategoryForClient();
        out.writeLine("ok");

        out.write("creating and adding callback receiver object... ");
        callbackReceiverImpl = new CallbackReceiverI();
        callbackReceiver = callbackReceiverImpl;

        let callbackReceiverIdent = new Ice.Identity();
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = category;
        twowayR = CallbackReceiverPrx.uncheckedCast(adapter.add(callbackReceiver, callbackReceiverIdent));

        let fakeCallbackReceiverIdent = new Ice.Identity();
        fakeCallbackReceiverIdent.name = "callbackReceiver";
        fakeCallbackReceiverIdent.category = "dummy";
        fakeTwowayR = CallbackReceiverPrx.uncheckedCast(adapter.add(callbackReceiver, fakeCallbackReceiverIdent));
        out.writeLine("ok");

        out.write("testing oneway callback... ");
        oneway = CallbackPrx.uncheckedCast(twoway.ice_oneway());
        onewayR = CallbackReceiverPrx.uncheckedCast(twowayR.ice_oneway());
        let context = new Ice.Context();
        context.set("_fwd", "o");
        await oneway.initiateCallback(onewayR, context);
        await callbackReceiverImpl.callbackOK();
        out.writeLine("ok");

        out.write("testing twoway callback... ");
        context = new Ice.Context();
        context.set("_fwd", "t");
        await twoway.initiateCallback(twowayR, context);
        await callbackReceiverImpl.callbackOK();
        out.writeLine("ok");

        out.write("ditto, but with user exception... ");
        context = new Ice.Context();
        context.set("_fwd", "t");
        try
        {
            await twoway.initiateCallbackEx(twowayR, context);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.CallbackException, ex);
            test(ex.someValue == 3.14);
            test(ex.someString == "3.14");
        }

        await callbackReceiverImpl.callbackOK();
        out.writeLine("ok");

        out.write("trying twoway callback with fake category... ");
        context = new Ice.Context();
        context.set("_fwd", "t");
        try
        {
            await twoway.initiateCallback(fakeTwowayR, context);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.ObjectNotExistException, ex);
        }
        out.writeLine("ok");

        out.write("testing whether other allowed category is accepted... ");
        context = new Ice.Context();
        context.set("_fwd", "t");
        let otherCategoryTwoway = CallbackPrx.uncheckedCast(twoway.ice_identity(Ice.stringToIdentity("c2/callback")));
        await otherCategoryTwoway.initiateCallback(twowayR, context);
        await callbackReceiverImpl.callbackOK();
        out.writeLine("ok");

        out.write("testing whether disallowed category gets rejected... ");
        context = new Ice.Context();
        context.set("_fwd", "t");
        otherCategoryTwoway = CallbackPrx.uncheckedCast(twoway.ice_identity(Ice.stringToIdentity("c3/callback")));
        try
        {
            await otherCategoryTwoway.initiateCallback(twowayR, context);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.ObjectNotExistException, ex);
        }
        out.writeLine("ok");

        out.write("testing whether user-id as category is accepted... ");
        context = new Ice.Context();
        context.set("_fwd", "t");
        otherCategoryTwoway = CallbackPrx.uncheckedCast(twoway.ice_identity(Ice.stringToIdentity("_userid/callback")));
        await otherCategoryTwoway.initiateCallback(twowayR, context);
        await callbackReceiverImpl.callbackOK();
        out.writeLine("ok");

        if(process.argv.indexOf("--shutdown") > -1)
        {
            out.write("testing server shutdown... ");
            await twoway.shutdown();
            out.writeLine("ok");
            // No ping, otherwise the router prints a warning message if it's
            // started with --Ice.Warn.Connections.
        }

        out.write("destroying session... ");
        await router.destroySession();
        out.writeLine("ok");

        out.write("trying to ping server after session destruction... ");
        try
        {
            await base.ice_ping();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.ConnectionLostException, ex);
        }
        out.writeLine("ok");

        if(process.argv.indexOf("--shutdown") > -1)
        {
            out.write("uninstalling router with communicator... ");
            communicator.setDefaultRouter(null);
            out.writeLine("ok");

            out.write("testing stringToProxy for process object... ");
            processBase = communicator.stringToProxy("Glacier2/admin -f Process:default -p 12061");
            out.writeLine("ok");

            out.write("testing checked cast for admin object... ");
            processPrx = await Ice.ProcessPrx.checkedCast(processBase);
            test(processPrx !== null);
            out.writeLine("ok");

            out.write("testing Glacier2 shutdown... ");
            await processPrx.shutdown();
            try
            {
                await processPrx.ice_timeout(500).ice_ping();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.LocalException, ex);
            }
            out.writeLine("ok");
        }
    }

    async function run(out, initData)
    {
        let communicator;
        try
        {
            initData.properties.setProperty("Ice.Warn.Dispatch", "1");
            initData.properties.setProperty("Ice.Warn.Connections", "0");
            communicator= Ice.initialize(initData);
            await allTests(out, communicator);
        }
        finally
        {
            if(communicator)
            {
                await communicator.shutdown();
            }
        }
    }

    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
