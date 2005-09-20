// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Glacier2;
using Test;

public class Client : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter;

        {
            adapter = communicator().createObjectAdapter("CallbackReceiverAdapter");
            adapter.activate();
            // Put the print statement after activate(), so that if
            // Ice.PrintAdapterReady is set, the "ready" is the first
            // output from the client, and not the print statement
            // below. Otherwise the Python test scripts will be confused,
            // as they expect the "ready" from the Object Adapter to be
            // the first thing that is printed.
            Console.Out.WriteLine("creating and activating callback receiver adapter... ok");
        }

        Ice.ObjectPrx routerBase;

        {
            Console.Out.Write("testing stringToProxy for router... ");
            Console.Out.Flush();
            routerBase = communicator().stringToProxy("abc/def:default -p 12347 -t 30000");
            Console.Out.WriteLine("ok");
        }
        
        Glacier2.RouterPrx router;

        {
            Console.Out.Write("testing checked cast for router... ");
            Console.Out.Flush();
            router = Glacier2.RouterPrxHelper.checkedCast(routerBase);
            test(router != null);
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("installing router with communicator... ");
            Console.Out.Flush();
            communicator().setDefaultRouter(router);
            Console.Out.WriteLine("ok");
        }

        Ice.ObjectPrx @base;

        {
            Console.Out.Write("testing stringToProxy for server object... ");
            Console.Out.Flush();
            @base = communicator().stringToProxy("c1/callback:tcp -p 12345 -t 10000");
            Console.Out.WriteLine("ok");
        }
            
        {
            Console.Out.Write("trying to ping server before session creation... ");
            Console.Out.Flush();
            try
            {
                @base.ice_ping();
                test(false);
            }
            catch(Ice.ConnectionLostException ex)
            {
                Console.Out.WriteLine("ok");
            }
            catch(Ice.SocketException ex)
            {
		test(false);
            }
        }


        {
            Console.Out.Write("trying to create session with wrong password... ");
            Console.Out.Flush();
            try
            {
                router.createSession("userid", "xxx");
                test(false);
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                Console.Out.WriteLine("ok");
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                test(false);
            }
        }

        {
            Console.Out.Write("trying to destroy non-existing session... ");
            Console.Out.Flush();
            try
            {
                router.destroySession();
                test(false);
            }
            catch(Glacier2.SessionNotExistException ex)
            {
                Console.Out.WriteLine("ok");
            }
        }

        {
            Console.Out.Write("creating session with correct password... ");
            Console.Out.Flush();
            try
            {
                router.createSession("userid", "abc123");
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                test(false);
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                test(false);
            }
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("trying to create a second session... ");
            Console.Out.Flush();
            try
            {
                router.createSession("userid", "abc123");
                test(false);
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                test(false);
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                Console.Out.WriteLine("ok");
            }
        }

        {
            Console.Out.Write("pinging server after session creation... ");
            Console.Out.Flush();
            @base.ice_ping();
            Console.Out.WriteLine("ok");
        }

        CallbackPrx twoway;

        {
            Console.Out.Write("testing checked cast for server object... ");
            Console.Out.Flush();
            twoway = CallbackPrxHelper.checkedCast(@base);
            test(twoway != null);
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("installing router with object adapter... ");
            Console.Out.Flush();
            adapter.addRouter(router);
            Console.Out.WriteLine("ok");
        }

        String category;

        {
            Console.Out.Write("getting category from router... ");
            Console.Out.Flush();
            category = router.getServerProxy().ice_getIdentity().category;
            Console.Out.WriteLine("ok");
        }

        CallbackReceiverI callbackReceiverImpl;
        Ice.Object callbackReceiver;
        CallbackReceiverPrx twowayR;
        CallbackReceiverPrx fakeTwowayR;
        
        {
            Console.Out.Write("creating and adding callback receiver object... ");
            Console.Out.Flush();
            callbackReceiverImpl = new CallbackReceiverI();
            callbackReceiver = callbackReceiverImpl;
            Ice.Identity callbackReceiverIdent = new Ice.Identity();
            callbackReceiverIdent.name = "callbackReceiver";
            callbackReceiverIdent.category = category;
            twowayR = CallbackReceiverPrxHelper.uncheckedCast(adapter.add(callbackReceiver, callbackReceiverIdent));
            Ice.Identity fakeCallbackReceiverIdent = new Ice.Identity();
            fakeCallbackReceiverIdent.name = "callbackReceiver";
            fakeCallbackReceiverIdent.category = "dummy";
            fakeTwowayR = CallbackReceiverPrxHelper.uncheckedCast(
                adapter.add(callbackReceiver, fakeCallbackReceiverIdent));
            Console.Out.WriteLine("ok");
        }
        
        {
            Console.Out.Write("testing oneway callback... ");
            Console.Out.Flush();
            CallbackPrx oneway = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway());
            CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway());
            Ice.Context context = new Ice.Context();
            context["_fwd"] =  "o";
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

        {
            Console.Out.Write("trying twoway callback with fake category... ");
            Console.Out.Flush();
            Ice.Context context = new Ice.Context();
            context["_fwd"] = "t";
            try
            {
                twoway.initiateCallback(fakeTwowayR, context);
                test(false);
            }
            catch(Ice.ObjectNotExistException ex)
            {
                Console.Out.WriteLine("ok");
            }
        }

	{
	    Console.Out.Write("testing whether other allowed category is accepted... ");
            Console.Out.Flush();
            Ice.Context context = new Ice.Context();
            context["_fwd"] =  "t";
	    CallbackPrx otherCategoryTwoway = CallbackPrxHelper.uncheckedCast(
		twoway.ice_newIdentity(Ice.Util.stringToIdentity("c2/callback")));
	    otherCategoryTwoway.initiateCallback(twowayR, context);
	    test(callbackReceiverImpl.callbackOK());
	    Console.Out.WriteLine("ok");
	}
	
	{
	    Console.Out.Write("testing whether disallowed category gets rejected... ");
            Console.Out.Flush();
            Ice.Context context = new Ice.Context();
            context["_fwd"] = "t";
	    try
	    {
		CallbackPrx otherCategoryTwoway = CallbackPrxHelper.uncheckedCast(
		    twoway.ice_newIdentity(Ice.Util.stringToIdentity("c3/callback")));
		otherCategoryTwoway.initiateCallback(twowayR, context);
		test(false);
	    }
	    catch(Ice.ObjectNotExistException ex)
	    {
		Console.Out.WriteLine("ok");
	    }
	}
	
	{
	    Console.Out.Write("testing whether user-id as category is accepted... ");
            Console.Out.Flush();
            Ice.Context context = new Ice.Context();
            context["_fwd"] = "t";
	    CallbackPrx otherCategoryTwoway = CallbackPrxHelper.uncheckedCast(
		twoway.ice_newIdentity(Ice.Util.stringToIdentity("_userid/callback")));
	    otherCategoryTwoway.initiateCallback(twowayR, context);
	    test(callbackReceiverImpl.callbackOK());
	    Console.Out.WriteLine("ok");
	}
	
        {
            Console.Out.Write("testing server shutdown... ");
            Console.Out.Flush();
            twoway.shutdown();
            // No ping, otherwise the router prints a warning message if it's
            // started with --Ice.Warn.Connections.
            Console.Out.WriteLine("ok");
            /*
              try
              {
                  base.ice_ping();
                  test(false);
              }
              // If we use the glacier router, the exact exception reason gets
              // lost.
              catch(Ice.UnknownLocalException ex)
              {
                  Console.Out.WriteLine("ok");
              }
            */
        }
        
        {
            Console.Out.Write("destroying session... ");
            Console.Out.Flush();
            try
            {
                router.destroySession();
            }
            catch(Glacier2.SessionNotExistException ex)
            {
                test(false);
            }
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("removing router from object adapter... ");
            Console.Out.Flush();
            adapter.removeRouter(router);
            Console.Out.WriteLine("ok");
        }
        
        {
            Console.Out.Write("trying to ping server after session destruction... ");
            Console.Out.Flush();
            try
            {
                @base.ice_ping();
                test(false);
            }
            catch(Ice.ConnectionLostException ex)
            {
                Console.Out.WriteLine("ok");
            }
            catch(Ice.SocketException ex)
            {
		test(false);
            }
        }
	
	if(args.Length >= 1 && args[0].Equals("--shutdown"))
	{
	    {
		Console.Out.Write("uninstalling router with communicator... ");
		Console.Out.Flush();
		communicator().setDefaultRouter(null);
		Console.Out.WriteLine("ok");
	    }
	    
	    Ice.ObjectPrx adminBase;
	    
	    {
		Console.Out.Write("testing stringToProxy for admin object... ");
		adminBase = communicator().stringToProxy("ABC/DEF:tcp -h 127.0.0.1 -p 12348 -t 10000");
		Console.Out.WriteLine("ok");
	    }
	    
/*
	    {
		Console.Out.Write("uninstalling router with admin object... ");
		adminBase.ice_router(null);
		Console.Out.WriteLine("ok");
	    }
*/
	    
	    Glacier2.AdminPrx admin;
	    
	    {
		Console.Out.Write("testing checked cast for admin object... ");
		admin = Glacier2.AdminPrxHelper.checkedCast(adminBase);
		test(admin != null);
		Console.Out.WriteLine("ok");
	    }
	    
	    Console.Out.Write("testing Glacier2 shutdown... ");
	    admin.shutdown();
	    try
	    {
		admin.ice_ping();
		test(false);
	    }
	    catch(Ice.LocalException ex)
	    {
		Console.Out.WriteLine("ok");
	    }
	}
	
        return 0;
    }

    private static void
    test(bool b)
    {
        if(!b)
        {
	    throw new Exception();
        }
    }

    public static void Main(string[] args)
    {
	//
	// We must disable connection warnings, because we attempt to
	// ping the router before session establishment, as well as
	// after session destruction. Both will cause a
	// ConnectionLostException.
	//
	Ice.Properties properties = Ice.Util.getDefaultProperties(ref args);
	properties.setProperty("Ice.Warn.Connections", "0");

        Client app = new Client();
        int status = app.main(args);

	Environment.Exit(status);
    }
}
