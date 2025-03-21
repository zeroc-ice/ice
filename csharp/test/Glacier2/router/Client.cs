// Copyright (c) ZeroC, Inc.

using System.Reflection;
using Test;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        //
        // We must disable connection warnings, because we attempt to
        // ping the router before session establishment, as well as
        // after session destruction. Both will cause a
        // ConnectionLostException.
        //
        properties.setProperty("Ice.Warn.Connections", "0");
        using var communicator = initialize(properties);
        Ice.ObjectPrx routerBase;
        {
            Console.Out.Write("testing stringToProxy for router... ");
            Console.Out.Flush();
            routerBase = communicator.stringToProxy("Glacier2/router:" + getTestEndpoint(50));
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
            Console.Out.Write("testing router finder... ");
            Console.Out.Flush();
            Ice.RouterFinderPrx finder =
                Ice.RouterFinderPrxHelper.uncheckedCast(communicator.stringToProxy("Ice/RouterFinder:" +
                                                                                   getTestEndpoint(50)));
            test(finder.getRouter().ice_getIdentity().Equals(router.ice_getIdentity()));
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("installing router with communicator... ");
            Console.Out.Flush();
            communicator.setDefaultRouter(router);
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("getting the session timeout... ");
            Console.Out.Flush();
            long timeout = router.getSessionTimeout();
            test(timeout == 30);
            Console.Out.WriteLine("ok");
        }

        Ice.ObjectPrx @base;

        {
            Console.Out.Write("testing stringToProxy for server object... ");
            Console.Out.Flush();
            @base = communicator.stringToProxy("c1/callback:" + getTestEndpoint(0));
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
            catch (Ice.ConnectionLostException)
            {
                Console.Out.WriteLine("ok");
            }
            catch (Ice.SocketException)
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
            catch (Glacier2.PermissionDeniedException)
            {
                Console.Out.WriteLine("ok");
            }
            catch (Glacier2.CannotCreateSessionException)
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
            catch (Glacier2.SessionNotExistException)
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
            catch (Glacier2.PermissionDeniedException)
            {
                test(false);
            }
            catch (Glacier2.CannotCreateSessionException)
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
            catch (Glacier2.PermissionDeniedException)
            {
                test(false);
            }
            catch (Glacier2.CannotCreateSessionException)
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

        {
            Console.Out.Write("pinging object with client endpoint... ");
            Ice.ObjectPrx baseC = communicator.stringToProxy("collocated:" + getTestEndpoint(50));
            try
            {
                baseC.ice_ping();
            }
            catch (Ice.ObjectNotExistException)
            {
            }
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

        Ice.ObjectAdapter adapter;

        {
            Console.Out.Write("creating and activating callback receiver adapter... ");
            Console.Out.Flush();
            communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");
            adapter = communicator.createObjectAdapterWithRouter("CallbackReceiverAdapter", router);
            adapter.activate();
            Console.Out.WriteLine("ok");
        }

        string category;

        {
            Console.Out.Write("getting category from router... ");
            Console.Out.Flush();
            category = router.getCategoryForClient();
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
            var callbackReceiverIdent = new Ice.Identity("callbackReceiver", category);
            twowayR = CallbackReceiverPrxHelper.uncheckedCast(adapter.add(callbackReceiver, callbackReceiverIdent));
            var fakeCallbackReceiverIdent = new Ice.Identity("callbackReceiver", "dummy");
            fakeTwowayR =
                CallbackReceiverPrxHelper.uncheckedCast(adapter.add(callbackReceiver, fakeCallbackReceiverIdent));
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing oneway callback... ");
            Console.Out.Flush();
            CallbackPrx oneway = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway());
            CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway());
            Dictionary<string, string> context = new Dictionary<string, string>
            {
                ["_fwd"] = "o"
            };
            oneway.initiateCallback(onewayR, context);
            callbackReceiverImpl.callbackOK();
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing twoway callback... ");
            Console.Out.Flush();
            Dictionary<string, string> context = new Dictionary<string, string>
            {
                ["_fwd"] = "t"
            };
            twoway.initiateCallback(twowayR, context);
            callbackReceiverImpl.callbackOK();
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("ditto, but with user exception... ");
            Console.Out.Flush();
            Dictionary<string, string> context = new Dictionary<string, string>
            {
                ["_fwd"] = "t"
            };
            try
            {
                twoway.initiateCallbackEx(twowayR, context);
                test(false);
            }
            catch (CallbackException ex)
            {
                test(ex.someValue == 3.14);
                test(ex.someString == "3.14");
            }
            callbackReceiverImpl.callbackOK();
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("trying twoway callback with fake category... ");
            Console.Out.Flush();
            Dictionary<string, string> context = new Dictionary<string, string>
            {
                ["_fwd"] = "t"
            };
            try
            {
                twoway.initiateCallback(fakeTwowayR, context);
                test(false);
            }
            catch (Ice.ObjectNotExistException)
            {
                Console.Out.WriteLine("ok");
            }
        }

        {
            Console.Out.Write("testing whether other allowed category is accepted... ");
            Console.Out.Flush();
            Dictionary<string, string> context = new Dictionary<string, string>
            {
                ["_fwd"] = "t"
            };
            CallbackPrx otherCategoryTwoway =
                CallbackPrxHelper.uncheckedCast(twoway.ice_identity(Ice.Util.stringToIdentity("c2/callback")));
            otherCategoryTwoway.initiateCallback(twowayR, context);
            callbackReceiverImpl.callbackOK();
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing whether disallowed category gets rejected... ");
            Console.Out.Flush();
            Dictionary<string, string> context = new Dictionary<string, string>
            {
                ["_fwd"] = "t"
            };
            try
            {
                CallbackPrx otherCategoryTwoway =
                    CallbackPrxHelper.uncheckedCast(twoway.ice_identity(Ice.Util.stringToIdentity("c3/callback")));
                otherCategoryTwoway.initiateCallback(twowayR, context);
                test(false);
            }
            catch (Ice.ObjectNotExistException)
            {
                Console.Out.WriteLine("ok");
            }
        }

        {
            Console.Out.Write("testing whether user-id as category is accepted... ");
            Console.Out.Flush();
            Dictionary<string, string> context = new Dictionary<string, string>
            {
                ["_fwd"] = "t"
            };
            CallbackPrx otherCategoryTwoway =
                CallbackPrxHelper.uncheckedCast(twoway.ice_identity(Ice.Util.stringToIdentity("_userid/callback")));
            otherCategoryTwoway.initiateCallback(twowayR, context);
            callbackReceiverImpl.callbackOK();
            Console.Out.WriteLine("ok");
        }

        if (args.Length >= 1 && args[0] == "--shutdown")
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
            catch (Ice.LocalException)
            {
                test(false);
            }

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
            catch (Ice.ConnectionLostException)
            {
                Console.Out.WriteLine("ok");
            }
            catch (Ice.SocketException)
            {
                test(false);
            }
        }

        if (args.Length >= 1 && args[0] == "--shutdown")
        {
            {
                Console.Out.Write("uninstalling router with communicator... ");
                Console.Out.Flush();
                communicator.setDefaultRouter(null);
                Console.Out.WriteLine("ok");
            }

            Ice.ObjectPrx processBase;

            {
                Console.Out.Write("testing stringToProxy for admin object... ");
                processBase = communicator.stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
                Console.Out.WriteLine("ok");
            }

            /*
              {
              Console.Out.Write("uninstalling router with process object... ");
              processBase.ice_router(null);
              Console.Out.WriteLine("ok");
              }
            */

            Ice.ProcessPrx process;

            {
                Console.Out.Write("testing checked cast for process object... ");
                process = Ice.ProcessPrxHelper.checkedCast(processBase);
                test(process != null);
                Console.Out.WriteLine("ok");
            }

            Console.Out.Write("testing Glacier2 shutdown... ");
            process.shutdown();
            try
            {
                process.ice_ping();
                test(false);
            }
            catch (Ice.LocalException)
            {
                Console.Out.WriteLine("ok");
            }
        }
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
