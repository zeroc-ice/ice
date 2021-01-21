// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Ice;
using ZeroC.Test;

namespace ZeroC.Glacier2.Test.Router
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            IObjectPrx routerBase;
            {
                Output.Write("testing stringToProxy for router... ");
                Output.Flush();
                routerBase = IObjectPrx.Parse(GetTestProxy("Glacier2/router", 50), Communicator);
                Output.WriteLine("ok");
            }

            IRouterPrx? router;
            {
                Output.Write("testing checked cast for router... ");
                Output.Flush();
                router = routerBase.CheckedCast(IRouterPrx.Factory);
                Assert(router != null);
                Output.WriteLine("ok");
            }

            {
                Output.Write("testing router finder... ");
                Output.Flush();
                var finder = IRouterFinderPrx.Parse(GetTestProxy("Ice/RouterFinder", 50), Communicator);
                Assert(finder.GetRouter()!.Identity.Equals(router.Identity));
                Output.WriteLine("ok");
            }

            {
                Output.Write("installing router with communicator... ");
                Output.Flush();
                Communicator.DefaultRouter = router;
                Output.WriteLine("ok");
            }

            {
                Output.Write("getting the session timeout... ");
                Output.Flush();
                long timeout = router.GetACMTimeout();
                Assert(timeout == 30);
                Output.WriteLine("ok");
            }

            ICallbackPrx twoway;
            {
                Output.Write("testing stringToProxy for server object... ");
                Output.Flush();
                twoway = ICallbackPrx.Parse(GetTestProxy("c1/callback", 0), Communicator);
                Output.WriteLine("ok");
            }

            {
                Output.Write("trying to ping server before session creation... ");
                Output.Flush();
                try
                {
                    twoway.IcePing();
                    Assert(false);
                }
                catch (ConnectionLostException)
                {
                    Output.WriteLine("ok");
                }
                catch (TransportException)
                {
                    Assert(false);
                }
            }

            {
                Output.Write("trying to create session with wrong password... ");
                Output.Flush();
                try
                {
                    router.CreateSession("userid", "xxx");
                    Assert(false);
                }
                catch (PermissionDeniedException)
                {
                    Output.WriteLine("ok");
                }
                catch (CannotCreateSessionException)
                {
                    Assert(false);
                }
            }

            {
                Output.Write("trying to destroy non-existing session... ");
                Output.Flush();
                try
                {
                    router.DestroySession();
                    Assert(false);
                }
                catch (SessionNotExistException)
                {
                    Output.WriteLine("ok");
                }
            }

            {
                Output.Write("creating session with correct password... ");
                Output.Flush();
                try
                {
                    router.CreateSession("userid", "abc123");
                }
                catch (PermissionDeniedException)
                {
                    Assert(false);
                }
                catch (CannotCreateSessionException)
                {
                    Assert(false);
                }
                Output.WriteLine("ok");
            }

            {
                Output.Write("trying to create a second session... ");
                Output.Flush();
                try
                {
                    router.CreateSession("userid", "abc123");
                    Assert(false);
                }
                catch (PermissionDeniedException)
                {
                    Assert(false);
                }
                catch (CannotCreateSessionException)
                {
                    Output.WriteLine("ok");
                }
            }

            {
                Output.Write("pinging server after session creation... ");
                Output.Flush();
                twoway.IcePing();
                Output.WriteLine("ok");
            }

            {
                Output.Write("pinging object with client endpoint... ");
                var baseC = IObjectPrx.Parse(GetTestProxy("collocated", 50), Communicator);
                try
                {
                    baseC.IcePing();
                }
                catch (ObjectNotExistException)
                {
                }
                Output.WriteLine("ok");
            }

            ObjectAdapter adapter;

            {
                Output.Write("creating and activating callback receiver adapter... ");
                Output.Flush();
                Communicator.SetProperty("Ice.PrintAdapterReady", "0");
                adapter = Communicator.CreateObjectAdapterWithRouter("CallbackReceiverAdapter", router);
                await adapter.ActivateAsync();
                Output.WriteLine("ok");
            }

            string category;

            {
                Output.Write("getting category from router... ");
                Output.Flush();
                category = router.GetCategoryForClient();
                Output.WriteLine("ok");
            }

            CallbackReceiver callbackReceiverImpl;
            ICallbackReceiver callbackReceiver;
            ICallbackReceiverPrx twowayR;
            ICallbackReceiverPrx fakeTwowayR;

            {
                Output.Write("creating and adding callback receiver object... ");
                Output.Flush();
                callbackReceiverImpl = new CallbackReceiver();
                callbackReceiver = callbackReceiverImpl;
                var callbackReceiverIdent = new Identity("callbackReceiver", category);
                twowayR = adapter.Add(callbackReceiverIdent, callbackReceiver, ICallbackReceiverPrx.Factory);
                var fakeCallbackReceiverIdent = new Identity("callbackReceiver", "dummy");
                fakeTwowayR = adapter.Add(fakeCallbackReceiverIdent, callbackReceiver,
                                            ICallbackReceiverPrx.Factory);
                Output.WriteLine("ok");
            }

            {
                Output.Write("testing oneway callback... ");
                Output.Flush();
                ICallbackPrx oneway = twoway.Clone(oneway: true);
                ICallbackReceiverPrx onewayR = twowayR.Clone(oneway: true);
                var context = new Dictionary<string, string>
                {
                    ["_fwd"] = "o"
                };
                oneway.InitiateCallback(onewayR, context);
                callbackReceiverImpl.CallbackOK();
                Output.WriteLine("ok");
            }

            {
                Output.Write("testing twoway callback... ");
                Output.Flush();
                var context = new Dictionary<string, string>
                {
                    ["_fwd"] = "t"
                };
                twoway.InitiateCallback(twowayR, context);
                callbackReceiverImpl.CallbackOK();
                Output.WriteLine("ok");
            }

            {
                Output.Write("ditto, but with user exception... ");
                Output.Flush();
                var context = new Dictionary<string, string>
                {
                    ["_fwd"] = "t"
                };
                try
                {
                    twoway.InitiateCallbackEx(twowayR, context);
                    Assert(false);
                }
                catch (CallbackException ex)
                {
                    Assert(ex.SomeValue == 3.14);
                    Assert(ex.SomeString.Equals("3.14"));
                }
                callbackReceiverImpl.CallbackOK();
                Output.WriteLine("ok");
            }

            {
                Output.Write("trying twoway callback with fake category... ");
                Output.Flush();
                var context = new Dictionary<string, string>
                {
                    ["_fwd"] = "t"
                };
                try
                {
                    twoway.InitiateCallback(fakeTwowayR, context);
                    Assert(false);
                }
                catch (ObjectNotExistException)
                {
                    Output.WriteLine("ok");
                }
            }

            {
                Output.Write("testing whether other allowed category is accepted... ");
                Output.Flush();
                var context = new Dictionary<string, string>
                {
                    ["_fwd"] = "t"
                };
                ICallbackPrx otherCategoryTwoway = twoway.Clone(ICallbackPrx.Factory,
                                                                identity: Identity.Parse("c2/callback"));
                otherCategoryTwoway.InitiateCallback(twowayR, context);
                callbackReceiverImpl.CallbackOK();
                Output.WriteLine("ok");
            }

            {
                Output.Write("testing whether disallowed category gets rejected... ");
                Output.Flush();
                var context = new Dictionary<string, string>
                {
                    ["_fwd"] = "t"
                };
                try
                {
                    ICallbackPrx otherCategoryTwoway = twoway.Clone(ICallbackPrx.Factory,
                                                                    identity: Identity.Parse("c3/callback"));
                    otherCategoryTwoway.InitiateCallback(twowayR, context);
                    Assert(false);
                }
                catch (ObjectNotExistException)
                {
                    Output.WriteLine("ok");
                }
            }

            {
                Output.Write("testing whether user-id as category is accepted... ");
                Output.Flush();
                var context = new Dictionary<string, string>
                {
                    ["_fwd"] = "t"
                };
                ICallbackPrx otherCategoryTwoway = twoway.Clone(ICallbackPrx.Factory,
                                                                identity: Identity.Parse("_userid/callback"));
                otherCategoryTwoway.InitiateCallback(twowayR, context);
                callbackReceiverImpl.CallbackOK();
                Output.WriteLine("ok");
            }

            if (args.Length >= 1 && args[0].Equals("--shutdown"))
            {
                Output.Write("testing server shutdown... ");
                Output.Flush();
                twoway.Shutdown();
                // No ping, otherwise the router prints a warning message if it's
                // started with --Ice.Warn.Connections.
                Output.WriteLine("ok");
                /*
                try
                {
                base.IcePing();
                Assert(false);
                }
                // If we use the glacier router, the exact exception reason gets
                // lost.
                catch(Ice.UnknownLocalException ex)
                {
                Output.WriteLine("ok");
                }
                */
            }

            {
                Output.Write("destroying session... ");
                Output.Flush();
                try
                {
                    router.DestroySession();
                }
                catch
                {
                    Assert(false);
                }

                Output.WriteLine("ok");
            }

            {
                Output.Write("trying to ping server after session destruction... ");
                Output.Flush();
                try
                {
                    twoway.IcePing();
                    Assert(false);
                }
                catch (ConnectionLostException)
                {
                    Output.WriteLine("ok");
                }
                catch (TransportException)
                {
                    Assert(false);
                }
            }

            if (args.Length >= 1 && args[0].Equals("--shutdown"))
            {
                {
                    Output.Write("uninstalling router with communicator... ");
                    Output.Flush();
                    Communicator.DefaultRouter = null;
                    Output.WriteLine("ok");
                }

                IProcessPrx process;
                {
                    Output.Write("testing stringToProxy for admin object... ");
                    process = IProcessPrx.Parse(GetTestProxy("Glacier2/admin -f Process", 51), Communicator);
                    Output.WriteLine("ok");
                }

                /*
                {
                Output.Write("uninstalling router with process object... ");
                processBase.ice_router(null);
                Output.WriteLine("ok");
                }
                */

                Output.Write("testing Glacier2 shutdown... ");
                process.Shutdown();
                try
                {
                    process.IcePing();
                    Assert(false);
                }
                catch
                {
                    Output.WriteLine("ok");
                }
            }
        }

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            // We must disable connection warnings, because we attempt to ping the router before session establishment,
            // as well as after session destruction. Both will cause a ConnectionLostException.
            properties["Ice.Warn.Connections"] = "0";
            properties["Test.Protocol"] = "ice1";

            await using var communicator = CreateCommunicator(properties);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
