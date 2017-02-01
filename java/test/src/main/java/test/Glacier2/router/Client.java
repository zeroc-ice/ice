// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Glacier2.router;

import java.io.PrintWriter;

import test.Glacier2.router.Test.CallbackException;
import test.Glacier2.router.Test.CallbackPrx;
import test.Glacier2.router.Test.CallbackReceiverPrx;

public class Client extends test.Util.Application
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public int run(String[] args)
    {
        com.zeroc.Ice.ObjectPrx routerBase;

        PrintWriter out = getWriter();
        {
            out.print("testing stringToProxy for router... ");
            out.flush();
            routerBase = communicator().stringToProxy("Glacier2/router:" + getTestEndpoint(10));
            out.println("ok");
        }

        com.zeroc.Glacier2.RouterPrx router;

        {
            out.print("testing checked cast for router... ");
            out.flush();
            router = com.zeroc.Glacier2.RouterPrx.checkedCast(routerBase);
            test(router != null);
            out.println("ok");
        }

        {
            out.print("testing router finder... ");
            out.flush();
            com.zeroc.Ice.RouterFinderPrx finder = com.zeroc.Ice.RouterFinderPrx.uncheckedCast(
                communicator().stringToProxy("Ice/RouterFinder:" + getTestEndpoint(10)));
            test(finder.getRouter().ice_getIdentity().equals(router.ice_getIdentity()));
            out.println("ok");
        }

        {
            out.print("installing router with communicator... ");
            out.flush();
            communicator().setDefaultRouter(router);
            out.println("ok");
        }

        {
            out.print("getting the session timeout... ");
            out.flush();
            long timeout = router.getSessionTimeout();
            test(timeout == 30);
            out.println("ok");
        }

        com.zeroc.Ice.ObjectPrx base;

        {
            out.print("testing stringToProxy for server object... ");
            out.flush();
            base = communicator().stringToProxy("c1/callback:" + getTestEndpoint(0));
            out.println("ok");
        }

        {
            out.print("trying to ping server before session creation... ");
            out.flush();
            try
            {
                base.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectionLostException ex)
            {
                out.println("ok");
            }
            catch(com.zeroc.Ice.SocketException ex)
            {
                //
                // The JSSE implementation in the AIX JDK appears to have a
                // bug that causes a "bad certificate" exception to be raised
                // when the connection is forcefully dropped and a retry occurs.
                //
                if(System.getProperty("os.name").equals("AIX"))
                {
                    out.println("ok");
                }
                else
                {
                    test(false);
                }
            }
        }

        {
            out.print("trying to create session with wrong password... ");
            out.flush();
            try
            {
                router.createSession("userid", "xxx");
                test(false);
            }
            catch(com.zeroc.Glacier2.PermissionDeniedException ex)
            {
                out.println("ok");
            }
            catch(com.zeroc.Glacier2.CannotCreateSessionException ex)
            {
                test(false);
            }
        }

        {
            out.print("trying to destroy non-existing session... ");
            out.flush();
            try
            {
                router.destroySession();
                test(false);
            }
            catch(com.zeroc.Glacier2.SessionNotExistException ex)
            {
                out.println("ok");
            }
        }

        {
            out.print("creating session with correct password... ");
            out.flush();
            try
            {
                router.createSession("userid", "abc123");
            }
            catch(com.zeroc.Glacier2.PermissionDeniedException ex)
            {
                test(false);
            }
            catch(com.zeroc.Glacier2.CannotCreateSessionException ex)
            {
                test(false);
            }
            out.println("ok");
        }

        {
            out.print("trying to create a second session... ");
            out.flush();
            try
            {
                router.createSession("userid", "abc123");
                test(false);
            }
            catch(com.zeroc.Glacier2.PermissionDeniedException ex)
            {
                test(false);
            }
            catch(com.zeroc.Glacier2.CannotCreateSessionException ex)
            {
                out.println("ok");
            }
        }

        {
            out.print("pinging server after session creation... ");
            out.flush();
            base.ice_ping();
            out.println("ok");
        }

        CallbackPrx twoway;

        {
            out.print("testing checked cast for server object... ");
            out.flush();
            twoway = CallbackPrx.checkedCast(base);
            test(twoway != null);
            out.println("ok");
        }

        com.zeroc.Ice.ObjectAdapter adapter;

        {
            out.print("creating and activating callback receiver adapter... ");
            out.flush();
            communicator().getProperties().setProperty("Ice.PrintAdapterReady", "0");
            adapter = communicator().createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
            adapter.activate();
            out.println("ok");
        }

        String category;

        {
            out.print("getting category from router... ");
            out.flush();
            category = router.getCategoryForClient();
            out.println("ok");
        }

        CallbackReceiverI callbackReceiverImpl;
        com.zeroc.Ice.Object callbackReceiver;
        CallbackReceiverPrx twowayR;
        CallbackReceiverPrx fakeTwowayR;

        {
            out.print("creating and adding callback receiver object... ");
            out.flush();
            callbackReceiverImpl = new CallbackReceiverI();
            callbackReceiver = callbackReceiverImpl;
            com.zeroc.Ice.Identity callbackReceiverIdent = new com.zeroc.Ice.Identity();
            callbackReceiverIdent.name = "callbackReceiver";
            callbackReceiverIdent.category = category;
            twowayR = CallbackReceiverPrx.uncheckedCast(adapter.add(callbackReceiver, callbackReceiverIdent));
            com.zeroc.Ice.Identity fakeCallbackReceiverIdent = new com.zeroc.Ice.Identity();
            fakeCallbackReceiverIdent.name = "callbackReceiver";
            fakeCallbackReceiverIdent.category = "dummy";
            fakeTwowayR = CallbackReceiverPrx.uncheckedCast(adapter.add(callbackReceiver, fakeCallbackReceiverIdent));
            out.println("ok");
        }

        {
            out.print("testing oneway callback... ");
            out.flush();
            CallbackPrx oneway = twoway.ice_oneway();
            CallbackReceiverPrx onewayR = twowayR.ice_oneway();
            java.util.Map<String, String> context = new java.util.HashMap<>();
            context.put("_fwd", "o");
            oneway.initiateCallback(onewayR, context);
            callbackReceiverImpl.callbackOK();
            out.println("ok");
        }

        {
            out.print("testing twoway callback... ");
            out.flush();
            java.util.Map<String, String> context = new java.util.HashMap<>();
            context.put("_fwd", "t");
            twoway.initiateCallback(twowayR, context);
            callbackReceiverImpl.callbackOK();
            out.println("ok");
        }

        {
            out.print("ditto, but with user exception... ");
            out.flush();
            java.util.Map<String, String> context = new java.util.HashMap<>();
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
            callbackReceiverImpl.callbackOK();
            out.println("ok");
        }

        {
            out.print("trying twoway callback with fake category... ");
            out.flush();
            java.util.Map<String, String> context = new java.util.HashMap<>();
            context.put("_fwd", "t");
            try
            {
                twoway.initiateCallback(fakeTwowayR, context);
                test(false);
            }
            catch(com.zeroc.Ice.ObjectNotExistException ex)
            {
                out.println("ok");
            }
        }

        {
            out.print("testing whether other allowed category is accepted... ");
            out.flush();
            java.util.Map<String, String> context = new java.util.HashMap<>();
            context.put("_fwd", "t");
            CallbackPrx otherCategoryTwoway = CallbackPrx.uncheckedCast(
                twoway.ice_identity(com.zeroc.Ice.Util.stringToIdentity("c2/callback")));
            otherCategoryTwoway.initiateCallback(twowayR, context);
            callbackReceiverImpl.callbackOK();
            out.println("ok");
        }

        {
            out.print("testing whether disallowed category gets rejected... ");
            out.flush();
            java.util.Map<String, String> context = new java.util.HashMap<>();
            context.put("_fwd", "t");
            try
            {
                CallbackPrx otherCategoryTwoway = CallbackPrx.uncheckedCast(
                    twoway.ice_identity(com.zeroc.Ice.Util.stringToIdentity("c3/callback")));
                otherCategoryTwoway.initiateCallback(twowayR, context);
                test(false);
            }
            catch(com.zeroc.Ice.ObjectNotExistException ex)
            {
                out.println("ok");
            }
        }

        {
            out.print("testing whether user-id as category is accepted... ");
            out.flush();
            java.util.Map<String, String> context = new java.util.HashMap<>();
            context.put("_fwd", "t");
            CallbackPrx otherCategoryTwoway = CallbackPrx.uncheckedCast(
                twoway.ice_identity(com.zeroc.Ice.Util.stringToIdentity("_userid/callback")));
            otherCategoryTwoway.initiateCallback(twowayR, context);
            callbackReceiverImpl.callbackOK();
            out.println("ok");
        }

        if(args.length >= 1 && args[0].equals("--shutdown"))
        {
            out.print("testing server shutdown... ");
            out.flush();
            twoway.shutdown();
            // No ping, otherwise the router prints a warning message if it's
            // started with --Ice.Warn.Connections.
            out.println("ok");
            /*
              try
              {
                  base.ice_ping();
                  test(false);
              }
              // If we use the glacier router, the exact exception reason gets
              // lost.
              catch(com.zeroc.Ice.UnknownLocalException ex)
              {
                  System.out.println("ok");
              }
            */
        }

        {
            out.print("destroying session... ");
            out.flush();
            try
            {
                router.destroySession();
            }
            catch(com.zeroc.Glacier2.SessionNotExistException ex)
            {
                test(false);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                test(false);
            }
            out.println("ok");
        }

        {
            out.print("trying to ping server after session destruction... ");
            out.flush();
            try
            {
                base.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectionLostException ex)
            {
                out.println("ok");
            }
            catch(com.zeroc.Ice.SocketException ex)
            {
                //
                // The JSSE implementation in the AIX JDK appears to have a
                // bug that causes a "bad certificate" exception to be raised
                // when the connection is forcefully dropped and a retry occurs.
                //
                if(System.getProperty("os.name").equals("AIX"))
                {
                    out.println("ok");
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
                out.print("uninstalling router with communicator... ");
                out.flush();
                communicator().setDefaultRouter(null);
                out.println("ok");
            }

            com.zeroc.Ice.ObjectPrx processBase;

            {
                out.print("testing stringToProxy for process object... ");
                processBase = communicator().stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(11));
                out.println("ok");
            }

/*
            {
                out.print("uninstalling router with process object... ");
                processBase.ice_router(null);
                out.println("ok");
            }
*/

            com.zeroc.Ice.ProcessPrx process;

            {
                out.print("testing checked cast for admin object... ");
                process = com.zeroc.Ice.ProcessPrx.checkedCast(processBase);
                test(process != null);
                out.println("ok");
            }

            out.print("testing Glacier2 shutdown... ");
            process.shutdown();
            try
            {
                process.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                out.println("ok");
            }
        }

        return 0;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Glacier2.router");
        return initData;
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }
}
