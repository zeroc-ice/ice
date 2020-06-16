//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using ZeroC.Ice;
using Test;

namespace ZeroC.IceGrid.Test.Simple
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            Console.Out.Write("testing stringToProxy... ");
            Console.Out.Flush();
            string rf = "test @ TestAdapter";
            var obj = ITestIntfPrx.Parse(rf, communicator);
            Console.Out.WriteLine("ok");

            Console.Out.Write("pinging server... ");
            Console.Out.Flush();
            obj.IcePing();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing locator finder... ");
            Identity finderId = new Identity("LocatorFinder", "Ice");
            ILocatorFinderPrx? finder =
                ILocatorFinderPrx.CheckedCast(communicator.DefaultLocator!.Clone(finderId, IObjectPrx.Factory));
            TestHelper.Assert(finder != null && finder.GetLocator() != null);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing discovery... ");
            {
                // Add test well-known object
                IRegistryPrx? registry = IRegistryPrx.Parse(
                    communicator.DefaultLocator!.Identity.Category + "/Registry", communicator);

                IAdminSessionPrx? session = registry.CreateAdminSession("foo", "bar");
                TestHelper.Assert(session != null);
                session.GetAdmin()!.AddObjectWithType(obj, "::Test");
                session.Destroy();

                //
                // Ensure the IceGrid discovery locator can discover the
                // registries and make sure locator requests are forwarded.
                //
                var properties = communicator.GetProperties();
                properties.Remove("Ice.Default.Locator");
                properties["Ice.Plugin.IceLocatorDiscovery"] = "Ice:ZeroC.IceLocatorDiscovery.PluginFactory";
                properties["IceLocatorDiscovery.Port"] = helper.GetTestPort(99).ToString();
                properties["AdapterForDiscoveryTest.AdapterId"] = "discoveryAdapter";
                properties["AdapterForDiscoveryTest.Endpoints"] = "default";

                var com = new Communicator(properties);
                TestHelper.Assert(com.DefaultLocator != null);
                IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                IObjectPrx.Parse("test", com).IcePing();

                TestHelper.Assert(com.DefaultLocator!.GetRegistry() != null);
                TestHelper.Assert(ZeroC.IceGrid.ILocatorPrx.UncheckedCast(com.DefaultLocator!).GetLocalRegistry() != null);
                TestHelper.Assert(ZeroC.IceGrid.ILocatorPrx.UncheckedCast(com.DefaultLocator!).GetLocalQuery() != null);

                ObjectAdapter adapter = com.CreateObjectAdapter("AdapterForDiscoveryTest");
                adapter.Activate();
                adapter.Deactivate();
                com.Destroy();

                //
                // Now, ensure that the IceGrid discovery locator correctly
                // handles failure to find a locator.
                //
                properties["IceLocatorDiscovery.InstanceName"] = "unknown";
                properties["IceLocatorDiscovery.RetryCount"] = "1";
                properties["IceLocatorDiscovery.Timeout"] = "100";
                com = new Communicator(properties);
                TestHelper.Assert(com.DefaultLocator != null);
                try
                {
                    IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                }
                catch (NoEndpointException)
                {
                }
                try
                {
                    IObjectPrx.Parse("test", com).IcePing();
                }
                catch (NoEndpointException)
                {
                }

                TestHelper.Assert(com.DefaultLocator!.GetRegistry() == null);
                TestHelper.Assert(ZeroC.IceGrid.ILocatorPrx.CheckedCast(com.DefaultLocator!) == null);
                try
                {
                    ZeroC.IceGrid.ILocatorPrx.UncheckedCast(com.DefaultLocator!).GetLocalRegistry();
                }
                catch (OperationNotExistException)
                {
                }

                adapter = com.CreateObjectAdapter("AdapterForDiscoveryTest");
                adapter.Activate();
                adapter.Deactivate();

                com.Destroy();

                string multicast;
                if (communicator.GetProperty("Ice.IPv6") == "1")
                {
                    multicast = "\"ff15::1\"";
                }
                else
                {
                    multicast = "239.255.0.1";
                }

                //
                // Test invalid lookup endpoints
                //
                properties = communicator.GetProperties();
                properties.Remove("Ice.Default.Locator");
                properties["Ice.Plugin.IceLocatorDiscovery"] = "Ice:ZeroC.IceLocatorDiscovery.PluginFactory";
                properties["IceLocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
                com = new Communicator(properties);
                TestHelper.Assert(com.DefaultLocator != null);
                try
                {
                    IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                    TestHelper.Assert(false);
                }
                catch (NoEndpointException)
                {
                }
                com.Destroy();

                properties = communicator.GetProperties();
                properties.Remove("Ice.Default.Locator");
                properties["IceLocatorDiscovery.RetryCount"] = "0";
                properties["Ice.Plugin.IceLocatorDiscovery"] = "Ice:ZeroC.IceLocatorDiscovery.PluginFactory";
                properties["IceLocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
                com = new Communicator(properties);
                TestHelper.Assert(com.DefaultLocator != null);
                try
                {
                    IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                    TestHelper.Assert(false);
                }
                catch (NoEndpointException)
                {
                }
                com.Destroy();

                properties = communicator.GetProperties();
                properties.Remove("Ice.Default.Locator");
                properties["IceLocatorDiscovery.RetryCount"] = "1";
                properties["Ice.Plugin.IceLocatorDiscovery"] = "Ice:ZeroC.IceLocatorDiscovery.PluginFactory";
                {
                    string intf = communicator.GetProperty("IceLocatorDiscovery.Interface") ?? "";
                    if (intf != "")
                    {
                        intf = $" --interface \"{intf}\"";
                    }
                    string port = helper.GetTestPort(99).ToString();
                    properties["IceLocatorDiscovery.Lookup"] =
                        $"udp -h {multicast} --interface unknown:udp -h {multicast} -p {port}{intf}";
                }
                com = new Communicator(properties);
                TestHelper.Assert(com.DefaultLocator != null);
                try
                {
                    IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                }
                catch (NoEndpointException)
                {
                    TestHelper.Assert(false);
                }
                com.Destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("shutting down server... ");
            Console.Out.Flush();
            obj.shutdown();
            Console.Out.WriteLine("ok");
        }

        public static void
        allTestsWithDeploy(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            Console.Out.Write("testing stringToProxy... ");
            Console.Out.Flush();
            var obj = ITestIntfPrx.Parse("test @ TestAdapter", communicator);
            var obj2 = ITestIntfPrx.Parse("test", communicator);
            Console.Out.WriteLine("ok");

            Console.Out.Write("pinging server... ");
            Console.Out.Flush();
            obj.IcePing();
            obj2.IcePing();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing encoding versioning... ");
            Console.Out.Flush();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing reference with unknown identity... ");
            Console.Out.Flush();
            try
            {
                IObjectPrx.Parse("unknown/unknown", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (ObjectNotFoundException)
            {
                // expected
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing reference with unknown adapter... ");
            Console.Out.Flush();
            try
            {
                IObjectPrx.Parse("test @ TestAdapterUnknown", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (AdapterNotFoundException)
            {
                // expected
            }
            Console.Out.WriteLine("ok");

            IRegistryPrx registry = IRegistryPrx.Parse(
                communicator.DefaultLocator!.Identity.Category + "/Registry", communicator);
            IAdminSessionPrx? session = registry.CreateAdminSession("foo", "bar");
            TestHelper.Assert(session != null);
            session.GetConnection()!.SetACM(registry.GetACMTimeout(), null, ACMHeartbeat.HeartbeatAlways);

            IAdminPrx? admin = session.GetAdmin();
            TestHelper.Assert(admin != null);
            admin.EnableServer("server", false);
            admin.StopServer("server");

            Console.Out.Write("testing whether server is still reachable... ");
            Console.Out.Flush();
            try
            {
                obj.IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
            }
            try
            {
                obj2.IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
            }

            try
            {
                admin.EnableServer("server", true);
            }
            catch (ServerNotExistException)
            {
                TestHelper.Assert(false);
            }
            catch (NodeUnreachableException)
            {
                TestHelper.Assert(false);
            }

            try
            {
                obj.IcePing();
            }
            catch (NoEndpointException)
            {
                TestHelper.Assert(false);
            }
            try
            {
                obj2.IcePing();
            }
            catch (NoEndpointException)
            {
                TestHelper.Assert(false);
            }
            Console.Out.WriteLine("ok");

            try
            {
                admin.StopServer("server");
            }
            catch (ServerNotExistException)
            {
                TestHelper.Assert(false);
            }
            catch (ServerStopException)
            {
                TestHelper.Assert(false);
            }
            catch (NodeUnreachableException)
            {
                TestHelper.Assert(false);
            }

            session.Destroy();
        }
    }
}
