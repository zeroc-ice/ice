// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

class CallbackClient : Ice.Application
{
    public override int run(string[] args)
    {
        string @ref;
        communicator().getProperties().setProperty("CallbackReceiverAdapter.Endpoints", "default");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackReceiverAdapter");
        adapter.activate();
        // Put the print statement after activate(), so that if
        // Ice.PrintAdapterReady is set, the "ready" is the first output
        // from the client, and not the print statement below. Otherwise
        // the Python test scripts will be confused, as they expect the
        // "ready" from the Object Adapter to be the first thing that is
        // printed.
        Console.Out.Write("creating and activating callback receiver adapter... ");
        Console.Out.Flush();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("creating and adding callback receiver object... ");
        Console.Out.Flush();
        CallbackReceiverI callbackReceiverImpl = new CallbackReceiverI();
        Ice.Object callbackReceiver = callbackReceiverImpl;
        adapter.add(callbackReceiver, Ice.Util.stringToIdentity("callbackReceiver"));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing stringToProxy for glacier starter... ");
        Console.Out.Flush();
        @ref = "Glacier/starter:default -p 12346 -t 30000";
        Ice.ObjectPrx starterBase = communicator().stringToProxy(@ref);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast for glacier starter... ");
        Console.Out.Flush();
        Glacier.StarterPrx starter = Glacier.StarterPrxHelper.checkedCast(starterBase);
        test(starter != null);
        Console.Out.WriteLine("ok");
        
        byte[] privateKey;
        byte[] publicKey;
        byte[] routerCert;
        
        Console.Out.Write("starting up glacier router... ");
        Console.Out.Flush();
        Glacier.RouterPrx router;
        try
        {
            router = starter.startRouter("dummy", "abc123", out privateKey, out publicKey, out routerCert);
        }
        catch(Glacier.CannotStartRouterException ex)
        {
            Console.Out.WriteLine(appName() + ": " + ex + ":\n" + ex.reason);
            return 1;
        }
        catch(Glacier.PermissionDeniedException ex)
        {
            Console.Out.WriteLine(appName() + ": " + ex);
            return 1;
        }
        
        test(router != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("pinging glacier router... ");
        Console.Out.Flush();
        router.ice_ping();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("installing glacier router... ");
        Console.Out.Flush();
        communicator().setDefaultRouter(router);
        adapter.addRouter(router);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        @ref = "callback:tcp -p 12345 -t 10000";
        Ice.ObjectPrx @base = communicator().stringToProxy(@ref);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        CallbackPrx twoway = CallbackPrxHelper.checkedCast(@base.ice_twoway().ice_timeout(-1).ice_secure(false));
        test(twoway != null);
        Console.Out.WriteLine("ok");
        
        CallbackReceiverPrx twowayR =
	    CallbackReceiverPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("callbackReceiver")));
        
        {
            Console.Out.Write("testing oneway callback... ");
            Console.Out.Flush();
            CallbackPrx oneway = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway());
            CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway());
            Ice.Context context = new Ice.Context();
            context["_fwd"] = "o";
            oneway.initiateCallback(onewayR, context);
            test(callbackReceiverImpl.callbackOK());
            Console.Out.WriteLine("ok");
        }
        
        {
            Console.Out.Write("testing twoway callback... ");
            Console.Out.Flush();
            Ice.Context context = new Ice.Context();
            context["_fwd"] = "t";
            twoway.initiateCallback(twowayR, context);
            test(callbackReceiverImpl.callbackOK());
            Console.Out.WriteLine("ok");
        }
        
        {
            Console.Out.Write("ditto, but with user exception... ");
            Console.Out.Flush();
            Ice.Context context = new Ice.Context();
            context["_fwd"] = "t";
            try
            {
                twoway.initiateCallbackEx(twowayR, context);
                test(false);
            }
            catch(CallbackException ex)
            {
                test(ex.someValue == 3.14);
                test(ex.someString.Equals("3.14"));
            }
            test(callbackReceiverImpl.callbackOK());
            Console.Out.WriteLine("ok");
        }
        
        Console.Out.Write("testing server shutdown... ");
        Console.Out.Flush();
        twoway.shutdown();
        // No ping, otherwise the glacier router prints a warning
        // message if it's started with --Ice.Warn.Connections.
        Console.Out.WriteLine("ok");
        /*
        try
        {
        twoway.ice_ping();
        test(false);
        }
        catch(Ice.LocalException ex)
        {
        Console.WriteLine("ok");
        }
        */
        
        Console.Out.Write("shutting down glacier router... ");
        Console.Out.Flush();
        router.shutdown();
        try
        {
            router.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }
        
        return 0;
    }
    
    private static void test(bool b)
    {
        if(!b)
        {
            throw new SystemException();
        }
    }
}
