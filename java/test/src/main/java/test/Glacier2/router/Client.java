// Copyright (c) ZeroC, Inc.

package test.Glacier2.router;

import com.zeroc.Glacier2.CannotCreateSessionException;
import com.zeroc.Glacier2.PermissionDeniedException;
import com.zeroc.Glacier2.RouterPrx;
import com.zeroc.Glacier2.SessionNotExistException;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.ProcessPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.RouterFinderPrx;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.Util;

import test.Glacier2.router.Test.CallbackException;
import test.Glacier2.router.Test.CallbackPrx;
import test.Glacier2.router.Test.CallbackReceiverPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Stream;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.Package.Test", "test.Glacier2.router");

        try (Communicator communicator = initialize(properties)) {
            ObjectPrx routerBase;

            PrintWriter out = getWriter();
            {
                out.print("testing stringToProxy for router... ");
                out.flush();
                routerBase = communicator.stringToProxy("Glacier2/router:" + getTestEndpoint(50));
                out.println("ok");
            }

            RouterPrx router;

            {
                out.print("testing checked cast for router... ");
                out.flush();
                router = RouterPrx.checkedCast(routerBase);
                test(router != null);
                out.println("ok");
            }

            {
                out.print("testing router finder... ");
                out.flush();
                var finder =
                    RouterFinderPrx.createProxy(
                        communicator, "Ice/RouterFinder:" + getTestEndpoint(50));
                test(finder.getRouter().ice_getIdentity().equals(router.ice_getIdentity()));
                out.println("ok");
            }

            {
                out.print("installing router with communicator... ");
                out.flush();
                communicator.setDefaultRouter(router);
                out.println("ok");
            }

            {
                out.print("getting the session timeout... ");
                out.flush();
                long timeout = router.getSessionTimeout();
                test(timeout == 30);
                out.println("ok");
            }

            ObjectPrx base;

            {
                out.print("testing stringToProxy for server object... ");
                out.flush();
                base = communicator.stringToProxy("c1/callback:" + getTestEndpoint(0));
                out.println("ok");
            }

            {
                out.print("trying to ping server before session creation... ");
                out.flush();
                try {
                    base.ice_ping();
                    test(false);
                } catch (ConnectionLostException ex) {
                    out.println("ok");
                } catch (SocketException ex) {
                    System.err.println(ex);
                    test(false);
                }
            }

            {
                out.print("trying to create session with wrong password... ");
                out.flush();
                try {
                    router.createSession("userid", "xxx");
                    test(false);
                } catch (PermissionDeniedException ex) {
                    out.println("ok");
                } catch (CannotCreateSessionException ex) {
                    test(false);
                }
            }

            {
                out.print("trying to destroy non-existing session... ");
                out.flush();
                try {
                    router.destroySession();
                    test(false);
                } catch (SessionNotExistException ex) {
                    out.println("ok");
                }
            }

            {
                out.print("creating session with correct password... ");
                out.flush();
                try {
                    router.createSession("userid", "abc123");
                } catch (PermissionDeniedException ex) {
                    test(false);
                } catch (CannotCreateSessionException ex) {
                    test(false);
                }
                out.println("ok");
            }

            {
                out.print("trying to create a second session... ");
                out.flush();
                try {
                    router.createSession("userid", "abc123");
                    test(false);
                } catch (PermissionDeniedException ex) {
                    test(false);
                } catch (CannotCreateSessionException ex) {
                    out.println("ok");
                }
            }

            {
                out.print("pinging server after session creation... ");
                out.flush();
                base.ice_ping();
                out.println("ok");
            }

            {
                out.print("pinging object with client endpoint... ");
                out.flush();
                ObjectPrx baseC =
                    communicator.stringToProxy("collocated:" + getTestEndpoint(50));
                try {
                    baseC.ice_ping();
                } catch (ObjectNotExistException ex) {}
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

            ObjectAdapter adapter;

            {
                out.print("creating and activating callback receiver adapter... ");
                out.flush();
                communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");
                adapter =
                    communicator.createObjectAdapterWithRouter(
                        "CallbackReceiverAdapter", router);
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
            Object callbackReceiver;
            CallbackReceiverPrx twowayR;
            CallbackReceiverPrx fakeTwowayR;

            {
                out.print("creating and adding callback receiver object... ");
                out.flush();
                callbackReceiverImpl = new CallbackReceiverI();
                callbackReceiver = callbackReceiverImpl;
                Identity callbackReceiverIdent = new Identity();
                callbackReceiverIdent.name = "callbackReceiver";
                callbackReceiverIdent.category = category;
                twowayR =
                    CallbackReceiverPrx.uncheckedCast(
                        adapter.add(callbackReceiver, callbackReceiverIdent));
                Identity fakeCallbackReceiverIdent = new Identity();
                fakeCallbackReceiverIdent.name = "callbackReceiver";
                fakeCallbackReceiverIdent.category = "dummy";
                fakeTwowayR =
                    CallbackReceiverPrx.uncheckedCast(
                        adapter.add(callbackReceiver, fakeCallbackReceiverIdent));
                out.println("ok");
            }

            {
                out.print("testing oneway callback... ");
                out.flush();
                CallbackPrx oneway = twoway.ice_oneway();
                CallbackReceiverPrx onewayR = twowayR.ice_oneway();
                Map<String, String> context = new HashMap<>();
                context.put("_fwd", "o");
                oneway.initiateCallback(onewayR, context);
                callbackReceiverImpl.callbackOK();
                out.println("ok");
            }

            {
                out.print("testing twoway callback... ");
                out.flush();
                Map<String, String> context = new HashMap<>();
                context.put("_fwd", "t");
                twoway.initiateCallback(twowayR, context);
                callbackReceiverImpl.callbackOK();
                out.println("ok");
            }

            {
                out.print("ditto, but with user exception... ");
                out.flush();
                Map<String, String> context = new HashMap<>();
                context.put("_fwd", "t");
                try {
                    twoway.initiateCallbackEx(twowayR, context);
                    test(false);
                } catch (CallbackException ex) {
                    test(ex.someValue == 3.14);
                    test("3.14".equals(ex.someString));
                }
                callbackReceiverImpl.callbackOK();
                out.println("ok");
            }

            {
                out.print("trying twoway callback with fake category... ");
                out.flush();
                Map<String, String> context = new HashMap<>();
                context.put("_fwd", "t");
                try {
                    twoway.initiateCallback(fakeTwowayR, context);
                    test(false);
                } catch (ObjectNotExistException ex) {
                    out.println("ok");
                }
            }

            {
                out.print("testing whether other allowed category is accepted... ");
                out.flush();
                Map<String, String> context = new HashMap<>();
                context.put("_fwd", "t");
                CallbackPrx otherCategoryTwoway =
                    CallbackPrx.uncheckedCast(
                        twoway.ice_identity(
                            Util.stringToIdentity("c2/callback")));
                otherCategoryTwoway.initiateCallback(twowayR, context);
                callbackReceiverImpl.callbackOK();
                out.println("ok");
            }

            {
                out.print("testing whether disallowed category gets rejected... ");
                out.flush();
                Map<String, String> context = new HashMap<>();
                context.put("_fwd", "t");
                try {
                    CallbackPrx otherCategoryTwoway =
                        CallbackPrx.uncheckedCast(
                            twoway.ice_identity(
                                Util.stringToIdentity("c3/callback")));
                    otherCategoryTwoway.initiateCallback(twowayR, context);
                    test(false);
                } catch (ObjectNotExistException ex) {
                    out.println("ok");
                }
            }

            {
                out.print("testing whether user-id as category is accepted... ");
                out.flush();
                Map<String, String> context = new HashMap<>();
                context.put("_fwd", "t");
                CallbackPrx otherCategoryTwoway =
                    CallbackPrx.uncheckedCast(
                        twoway.ice_identity(
                            Util.stringToIdentity("_userid/callback")));
                otherCategoryTwoway.initiateCallback(twowayR, context);
                callbackReceiverImpl.callbackOK();
                out.println("ok");
            }

            boolean shutdown = Stream.of(args).anyMatch(v -> "--shutdown".equals(v));
            if (shutdown) {
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
                  // If we use the glacier router, the exact exception reason gets lost.
                  catch(com.zeroc.Ice.UnknownLocalException ex)
                  {
                  System.out.println("ok");
                  }
                */
            }

            {
                out.print("destroying session... ");
                out.flush();
                try {
                    router.destroySession();
                } catch (SessionNotExistException ex) {
                    test(false);
                } catch (LocalException ex) {
                    test(false);
                }
                out.println("ok");
            }

            {
                out.print("trying to ping server after session destruction... ");
                out.flush();
                try {
                    base.ice_ping();
                    test(false);
                } catch (ConnectionLostException ex) {
                    out.println("ok");
                } catch (SocketException ex) {
                    test(false);
                }
            }

            if (shutdown) {
                {
                    out.print("uninstalling router with communicator... ");
                    out.flush();
                    communicator.setDefaultRouter(null);
                    out.println("ok");
                }

                ObjectPrx processBase;

                {
                    out.print("testing stringToProxy for process object... ");
                    processBase =
                        communicator.stringToProxy(
                            "Glacier2/admin -f Process:" + getTestEndpoint(51));
                    out.println("ok");
                }

                /*
                  {
                  out.print("uninstalling router with process object... ");
                  processBase.ice_router(null);
                  out.println("ok");
                  }
                */

                ProcessPrx process;

                {
                    out.print("testing checked cast for admin object... ");
                    process = ProcessPrx.checkedCast(processBase);
                    test(process != null);
                    out.println("ok");
                }

                out.print("testing Glacier2 shutdown... ");
                process.shutdown();
                try {
                    process.ice_ping();
                    test(false);
                } catch (LocalException ex) {
                    out.println("ok");
                }
            }
        }
    }
}
