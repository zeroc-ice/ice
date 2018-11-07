// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice, Glacier2} from "ice";
import {Test} from "./Callback";
import {TestHelper} from "../../../Common/TestHelper";

const test = TestHelper.test;
const CallbackPrx = Test.CallbackPrx;
const CallbackReceiverPrx = Test.CallbackReceiverPrx;

export class CallbackReceiverI extends Test.CallbackReceiver
{
    constructor()
    {
        super();
        this._callback = false;
        this._p = new Ice.Promise();
    }

    callback(current:Ice.Current):void
    {
        test(!this._callback);
        this._p.resolve();
    }

    callbackEx(current:Ice.Current):void
    {
        this.callback(current);
        throw new Test.CallbackException(3.14, "3.14");
    }

    callbackOK():PromiseLike<void>
    {
        return this._p.then(() =>
                            {
                                this._callback = false;
                                this._p = new Ice.Promise();
                            });
    }

    _p:Ice.Promise<void>;
    _callback:boolean;
}

export class Client extends TestHelper
{
    async allTests(shutdown:boolean)
    {
        const out = this.getWriter();
        const communicator = this.communicator();

        out.write("testing stringToProxy for router... ");
        const routerBase = communicator.stringToProxy("Glacier2/router:" + this.getTestEndpoint(50));
        test(routerBase !== null);
        out.writeLine("ok");

        out.write("testing checked cast for router... ");
        const router = await Glacier2.RouterPrx.checkedCast(routerBase);
        test(router !== null);
        out.writeLine("ok");

        out.write("installing router with communicator... ");
        communicator.setDefaultRouter(router);
        out.writeLine("ok");

        out.write("getting the session timeout... ");
        const timeout = await router.getSessionTimeout();
        test(timeout.toNumber() === 30);
        out.writeLine("ok");

        out.write("testing stringToProxy for server object... ");
        const base = communicator.stringToProxy("c1/callback:" + this.getTestEndpoint());
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
        await router.createSession("userid", "abc123");
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
            const baseC = communicator.stringToProxy("collocated:" + this.getTestEndpoint(50));
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
        const twoway = await Test.CallbackPrx.checkedCast(base);
        test(twoway !== null);
        out.writeLine("ok");

        out.write("creating and activating callback receiver adapter... ");
        communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");
        const adapter = await communicator.createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
        await adapter.activate();
        out.writeLine("ok");

        out.write("getting category from router... ");
        const category = await router.getCategoryForClient();
        out.writeLine("ok");

        out.write("creating and adding callback receiver object... ");
        const callbackReceiverImpl = new CallbackReceiverI();
        const callbackReceiver = callbackReceiverImpl;

        const callbackReceiverIdent = new Ice.Identity();
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = category;
        const twowayR = CallbackReceiverPrx.uncheckedCast(adapter.add(callbackReceiver, callbackReceiverIdent));

        const fakeCallbackReceiverIdent = new Ice.Identity();
        fakeCallbackReceiverIdent.name = "callbackReceiver";
        fakeCallbackReceiverIdent.category = "dummy";
        const fakeTwowayR = CallbackReceiverPrx.uncheckedCast(adapter.add(callbackReceiver, fakeCallbackReceiverIdent));
        out.writeLine("ok");

        out.write("testing oneway callback... ");
        const oneway = CallbackPrx.uncheckedCast(twoway.ice_oneway());
        const onewayR = CallbackReceiverPrx.uncheckedCast(twowayR.ice_oneway());
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

        if(shutdown)
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

        if(shutdown)
        {
            out.write("uninstalling router with communicator... ");
            communicator.setDefaultRouter(null);
            out.writeLine("ok");

            out.write("testing stringToProxy for process object... ");
            const processBase = communicator.stringToProxy("Glacier2/admin -f Process:" + this.getTestEndpoint(51));
            out.writeLine("ok");

            out.write("testing checked cast for admin object... ");
            const processPrx = await Ice.ProcessPrx.checkedCast(processBase);
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

    async run(args:string[])
    {
        let communicator:Ice.Communicator;
        try
        {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Warn.Dispatch", "1");
            properties.setProperty("Ice.Warn.Connections", "0");
            [communicator] = this.initialize(properties);
            await this.allTests(args.indexOf("--shutdown") > -1);
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
