// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class CallbackClient extends Ice.Application
{
    public int
    run(String[] args)
    {
        String ref;

        Ice.ObjectAdapter adapter =
	    communicator().createObjectAdapterWithEndpoints("CallbackReceiverAdapter", "default");
        adapter.activate();
	// Put the print statement after activate(), so that if
	// Ice.PrintAdapterReady is set, the "ready" is the first output
	// from the client, and not the print statement below. Otherwise
	// the Python test scripts will be confused, as they expect the
	// "ready" from the Object Adapter to be the first thing that is
	// printed.
        System.out.print("creating and activating callback receiver adapter... ");
        System.out.flush();
        System.out.println("ok");

        System.out.print("creating and adding callback receiver object... ");
        System.out.flush();
        CallbackReceiverI callbackReceiverImpl = new CallbackReceiverI();
        Ice.Object callbackReceiver = callbackReceiverImpl;
        adapter.add(callbackReceiver, Ice.Util.stringToIdentity("callbackReceiver"));
        System.out.println("ok");

        System.out.print("testing stringToProxy for glacier starter... ");
        System.out.flush();
        ref = "Glacier/starter:default -p 12346 -t 5000";
        Ice.ObjectPrx starterBase = communicator().stringToProxy(ref);
        System.out.println("ok");

        System.out.print("testing checked cast for glacier starter... ");
        System.out.flush();
        Glacier.StarterPrx starter = Glacier.StarterPrxHelper.checkedCast(starterBase);
        test(starter != null);
        System.out.println("ok");

        Ice.ByteSeqHolder privateKey = new Ice.ByteSeqHolder();
        Ice.ByteSeqHolder publicKey = new Ice.ByteSeqHolder();
        Ice.ByteSeqHolder routerCert = new Ice.ByteSeqHolder();

        System.out.print("starting up glacier router... ");
        System.out.flush();
        Glacier.RouterPrx router;
        try
        {
            router = starter.startRouter("dummy", "abc123", privateKey, publicKey, routerCert);
        }
        catch (Glacier.CannotStartRouterException ex)
        {
            System.out.println(appName() + ": " + ex + ":\n" + ex.reason);
            return 1;
        }
        catch (Glacier.InvalidPasswordException ex)
        {
            System.out.println(appName() + ": " + ex);
            return 1;
        }

        test(router != null);
        System.out.println("ok");

        System.out.print("pinging glacier router... ");
        System.out.flush();
        router.ice_ping();
        System.out.println("ok");

        System.out.print("installing glacier router... ");
        System.out.flush();
        communicator().setDefaultRouter(router);
        adapter.addRouter(router);
        System.out.println("ok");

        System.out.print("testing stringToProxy... ");
        System.out.flush();
        ref = "callback:tcp -p 12345 -t 5000";
        Ice.ObjectPrx base = communicator().stringToProxy(ref);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        CallbackPrx twoway = CallbackPrxHelper.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(false));
        test(twoway != null);
        System.out.println("ok");

        CallbackReceiverPrx twowayR = CallbackReceiverPrxHelper.uncheckedCast(
            adapter.createProxy(Ice.Util.stringToIdentity("callbackReceiver")));

        {
            System.out.print("testing oneway callback... ");
            System.out.flush();
            CallbackPrx oneway = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway());
            CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway());
            java.util.Map context = new java.util.HashMap();
            context.put("_fwd", "o");
            oneway.initiateCallback(onewayR, context);
            test(callbackReceiverImpl.callbackOK());
            System.out.println("ok");
        }

        {
            System.out.print("testing twoway callback... ");
            System.out.flush();
            java.util.Map context = new java.util.HashMap();
            context.put("_fwd", "t");
            twoway.initiateCallback(twowayR, context);
            test(callbackReceiverImpl.callbackOK());
            System.out.println("ok");
        }

        {
            System.out.print("ditto, but with user exception... ");
            System.out.flush();
            java.util.Map context = new java.util.HashMap();
            context.put("_fwd", "t");
            try
            {
                twoway.initiateCallbackEx(twowayR, context);
                test(false);
            }
            catch (CallbackException ex)
            {
                test(ex.someValue == 3.14);
                test(ex.someString.equals("3.14"));
            }
            test(callbackReceiverImpl.callbackOK());
            System.out.println("ok");
        }

        System.out.print("testing server shutdown... ");
        System.out.flush();
        twoway.shutdown();
        // No ping, otherwise the glacier router prints a warning
        // message if it's started with --Ice.ConnectionWarnings.
        System.out.println("ok");
        /*
        try
        {
            twoway.ice_ping();
            test(false);
        }
        // If we use the glacier router, the exact exception reason gets
        // lost.
        //catch (Ice.ConnectFailedException ex)
        catch (Ice.UnknownLocalException ex)
        {
            System.out.println("ok");
        }
        */

        System.out.print("shutting down glacier router... ");
        System.out.flush();
        router.shutdown();
        try
        {
            router.ice_ping();
            test(false);
        }
        catch (Ice.ConnectFailedException ex)
        {
            System.out.println("ok");
        }

        return 0;
    }

    private static void
    test(boolean b)
    {
        if (!b)
        {
            throw new RuntimeException();
        }
    }
}
