// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

class CallbackClient extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.ObjectPrx routerBase;

        {
            System.out.print("testing stringToProxy for router... ");
            System.out.flush();
            routerBase = communicator().stringToProxy("Glacier2/router:default -p 12347 -t 30000");
            System.out.println("ok");
        }
        
        Glacier2.RouterPrx router;

        {
            System.out.print("testing checked cast for router... ");
            System.out.flush();
            router = Glacier2.RouterPrxHelper.checkedCast(routerBase);
            test(router != null);
            System.out.println("ok");
        }

        {
            System.out.print("installing router with communicator... ");
            System.out.flush();
            communicator().setDefaultRouter(router);
            System.out.println("ok");
        }

        {
            System.out.print("getting the session timeout... ");
            System.out.flush();
            long timeout = router.getSessionTimeout();
            test(timeout == 30);
            System.out.println("ok");
        }

        Ice.ObjectPrx base;

        {
            System.out.print("testing stringToProxy for server object... ");
            System.out.flush();
            base = communicator().stringToProxy("c1/callback:tcp -p 12010 -t 10000");
            System.out.println("ok");
        }
            
        {
            System.out.print("trying to ping server before session creation... ");
            System.out.flush();
            try
            {
                base.ice_ping();
                test(false);
            }
            catch(Ice.ConnectionLostException ex)
            {
                System.out.println("ok");
            }
            catch(Ice.SocketException ex)
            {
                //
                // The JSSE implementation in the AIX JDK appears to have a
                // bug that causes a "bad certificate" exception to be raised
                // when the connection is forcefully dropped and a retry occurs.
                //
                if(System.getProperty("os.name").equals("AIX"))
                {
                    System.out.println("ok");
                }
                else
                {
                    test(false);
                }
            }
        }

        Glacier2.SessionPrx session;

        {
            System.out.print("trying to create session with wrong password... ");
            System.out.flush();
            try
            {
                session = router.createSession("userid", "xxx");
                test(false);
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                System.out.println("ok");
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                test(false);
            }
        }

        {
            System.out.print("trying to destroy non-existing session... ");
            System.out.flush();
            try
            {
                router.destroySession();
                test(false);
            }
            catch(Glacier2.SessionNotExistException ex)
            {
                System.out.println("ok");
            }
        }

        {
            System.out.print("creating session with correct password... ");
            System.out.flush();
            try
            {
                session = router.createSession("userid", "abc123");
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                test(false);
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                test(false);
            }
            System.out.println("ok");
        }

        {
            System.out.print("trying to create a second session... ");
            System.out.flush();
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
                System.out.println("ok");
            }
        }

        {
            System.out.print("pinging server after session creation... ");
            System.out.flush();
            base.ice_ping();
            System.out.println("ok");
        }

        CallbackPrx twoway;

        {
            System.out.print("testing checked cast for server object... ");
            System.out.flush();
            twoway = CallbackPrxHelper.checkedCast(base);
            test(twoway != null);
            System.out.println("ok");
        }

        Ice.ObjectAdapter adapter;

        {
            System.out.print("creating and activating callback receiver adapter... ");
            System.out.flush();
            communicator().getProperties().setProperty("Ice.PrintAdapterReady", "0");
            adapter = communicator().createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
            adapter.activate();
            System.out.println("ok");
        }

        String category;

        {
            System.out.print("getting category from router... ");
            System.out.flush();
            category = router.getCategoryForClient();
            System.out.println("ok");
        }

        CallbackReceiverI callbackReceiverImpl;
        Ice.Object callbackReceiver;
        CallbackReceiverPrx twowayR;
        CallbackReceiverPrx fakeTwowayR;
        
        {
            System.out.print("creating and adding callback receiver object... ");
            System.out.flush();
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
            System.out.println("ok");
        }
        
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
            catch(CallbackException ex)
            {
                test(ex.someValue == 3.14);
                test(ex.someString.equals("3.14"));
            }
            test(callbackReceiverImpl.callbackOK());
            System.out.println("ok");
        }

        {
            System.out.print("trying twoway callback with fake category... ");
            System.out.flush();
            java.util.Map context = new java.util.HashMap();
            context.put("_fwd", "t");
            try
            {
                twoway.initiateCallback(fakeTwowayR, context);
                test(false);
            }
            catch(Ice.ObjectNotExistException ex)
            {
                System.out.println("ok");
            }
        }

        {
            System.out.print("testing whether other allowed category is accepted... ");
            System.out.flush();
            java.util.Map context = new java.util.HashMap();
            context.put("_fwd", "t");
            CallbackPrx otherCategoryTwoway = CallbackPrxHelper.uncheckedCast(
                twoway.ice_identity(communicator().stringToIdentity("c2/callback")));
            otherCategoryTwoway.initiateCallback(twowayR, context);
            test(callbackReceiverImpl.callbackOK());
            System.out.println("ok");
        }
        
        {
            System.out.print("testing whether disallowed category gets rejected... ");
            System.out.flush();
            java.util.Map context = new java.util.HashMap();
            context.put("_fwd", "t");
            try
            {
                CallbackPrx otherCategoryTwoway = CallbackPrxHelper.uncheckedCast(
                    twoway.ice_identity(communicator().stringToIdentity("c3/callback")));
                otherCategoryTwoway.initiateCallback(twowayR, context);
                test(false);
            }
            catch(Ice.ObjectNotExistException ex)
            {
                System.out.println("ok");
            }
        }
        
        {
            System.out.print("testing whether user-id as category is accepted... ");
            System.out.flush();
            java.util.Map context = new java.util.HashMap();
            context.put("_fwd", "t");
            CallbackPrx otherCategoryTwoway = CallbackPrxHelper.uncheckedCast(
                twoway.ice_identity(communicator().stringToIdentity("_userid/callback")));
            otherCategoryTwoway.initiateCallback(twowayR, context);
            test(callbackReceiverImpl.callbackOK());
            System.out.println("ok");
        }
        
        {
            System.out.print("testing server shutdown... ");
            System.out.flush();
            twoway.shutdown();
            // No ping, otherwise the router prints a warning message if it's
            // started with --Ice.Warn.Connections.
            System.out.println("ok");
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
                  System.out.println("ok");
              }
            */
        }
        
        {
            System.out.print("destroying session... ");
            System.out.flush();
            try
            {
                router.destroySession();
                test(false);
            }
            catch(Glacier2.SessionNotExistException ex)
            {
                test(false);
            }
            catch(Ice.ConnectionLostException ex)
            {
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            System.out.println("ok");
        }
        
        {
            System.out.print("trying to ping server after session destruction... ");
            System.out.flush();
            try
            {
                base.ice_ping();
                test(false);
            }
            catch(Ice.ConnectionLostException ex)
            {
                System.out.println("ok");
            }
            catch(Ice.SocketException ex)
            {
                //
                // The JSSE implementation in the AIX JDK appears to have a
                // bug that causes a "bad certificate" exception to be raised
                // when the connection is forcefully dropped and a retry occurs.
                //
                if(System.getProperty("os.name").equals("AIX"))
                {
                    System.out.println("ok");
                }
                else
                {
                    test(false);
                }
            }
        }
        
        if(args.length >= 1 && args[0].equals("--shutdown"))
        {
            {
                System.out.print("uninstalling router with communicator... ");
                System.out.flush();
                communicator().setDefaultRouter(null);
                System.out.println("ok");
            }
            
            Ice.ObjectPrx adminBase;
            
            {
                System.out.print("testing stringToProxy for admin object... ");
                adminBase = communicator().stringToProxy("Glacier2/admin:tcp -h 127.0.0.1 -p 12348 -t 10000");
                System.out.println("ok");
            }
            
/*
            {
                System.out.print("uninstalling router with admin object... ");
                adminBase.ice_router(null);
                System.out.println("ok");
            }
*/
            
            Glacier2.AdminPrx admin;
            
            {
                System.out.print("testing checked cast for admin object... ");
                admin = Glacier2.AdminPrxHelper.checkedCast(adminBase);
                test(admin != null);
                System.out.println("ok");
            }
            
            System.out.print("testing Glacier2 shutdown... ");
            admin.shutdown();
            try
            {
                admin.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
                System.out.println("ok");
            }
        }
        
        return 0;
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }
}
