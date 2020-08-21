//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;
using ZeroC.Ice;

namespace ZeroC.IceGrid.Test.Simple
{
    public class AllTests
    {
        public static void Run(TestHelper helper)
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
            var finderId = new Identity("LocatorFinder", "Ice");
            ILocatorFinderPrx finder = communicator.DefaultLocator!.Clone(ILocatorFinderPrx.Factory, identity: finderId);
            TestHelper.Assert(finder != null && finder.GetLocator() != null);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing discovery... ");
            Console.Out.Flush();
            {
                // Add test well-known object
                var registry = IRegistryPrx.Parse(
                    communicator.DefaultLocator!.Identity.Category + "/Registry", communicator);

                IAdminSessionPrx? session = registry.CreateAdminSession("foo", "bar");
                TestHelper.Assert(session != null);
                session.GetAdmin()!.AddObjectWithType(obj, "::Test");
                session.Destroy();

                //
                // Ensure the IceGrid discovery locator can discover the
                // registries and make sure locator requests are forwarded.
                //
                Dictionary<string, string>? properties = communicator.GetProperties();
                properties.Remove("Ice.Default.Locator");
                properties["Ice.Plugin.IceLocatorDiscovery"] = "Ice:ZeroC.IceLocatorDiscovery.PluginFactory";
                properties["IceLocatorDiscovery.Port"] = helper.GetTestPort(99).ToString();
                properties["AdapterForDiscoveryTest.AdapterId"] = "discoveryAdapter";
                properties["AdapterForDiscoveryTest.Endpoints"] = "default -h 127.0.0.1";

                {
                    using var com = new Communicator(properties);
                    TestHelper.Assert(com.DefaultLocator != null);
                    IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                    IObjectPrx.Parse("test", com).IcePing();

                    // TODO: currently, com.DefaultLocator is a regular ice2/2.0 proxy and we don't want to forward
                    // 2.0-encoded requests to IceGrid until IceGrid supports such requests.

                    ILocatorPrx defaultLocator = com.DefaultLocator!.Clone(ILocatorPrx.Factory,
                                                                           encoding: Encoding.V1_1);

                    TestHelper.Assert(defaultLocator.GetRegistry() != null);
                    TestHelper.Assert(defaultLocator.GetLocalRegistry() != null);
                    TestHelper.Assert(defaultLocator.GetLocalQuery() != null);

                    ObjectAdapter adapter = com.CreateObjectAdapter("AdapterForDiscoveryTest");
                    adapter.Activate();
                }
                //
                // Now, ensure that the IceGrid discovery locator correctly
                // handles failure to find a locator.
                //
                {
                    properties["IceLocatorDiscovery.InstanceName"] = "unknown";
                    properties["IceLocatorDiscovery.RetryCount"] = "1";
                    properties["IceLocatorDiscovery.Timeout"] = "100ms";
                    using var com = new Communicator(properties);
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

                    Ice.ILocatorPrx defaultLocator = com.DefaultLocator!.Clone(encoding: Encoding.V1_1);

                    TestHelper.Assert(defaultLocator.GetRegistry() == null);
                    TestHelper.Assert(defaultLocator.CheckedCast(ILocatorPrx.Factory) == null);
                    try
                    {
                        com.DefaultLocator!.Clone(ILocatorPrx.Factory).GetLocalRegistry();
                    }
                    catch (OperationNotExistException)
                    {
                    }

                    using ObjectAdapter adapter = com.CreateObjectAdapter("AdapterForDiscoveryTest");
                    adapter.Activate();
                }

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
                {
                    properties = communicator.GetProperties();
                    properties.Remove("Ice.Default.Locator");
                    properties["Ice.Plugin.IceLocatorDiscovery"] = "Ice:ZeroC.IceLocatorDiscovery.PluginFactory";
                    properties["IceLocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
                    using var com = new Communicator(properties);
                    TestHelper.Assert(com.DefaultLocator != null);
                    try
                    {
                        IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (NoEndpointException)
                    {
                    }
                }

                {
                    properties = communicator.GetProperties();
                    properties.Remove("Ice.Default.Locator");
                    properties["IceLocatorDiscovery.RetryCount"] = "0";
                    properties["Ice.Plugin.IceLocatorDiscovery"] = "Ice:ZeroC.IceLocatorDiscovery.PluginFactory";
                    properties["IceLocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
                    using var com = new Communicator(properties);
                    TestHelper.Assert(com.DefaultLocator != null);
                    try
                    {
                        IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (NoEndpointException)
                    {
                    }
                }

                {
                    properties = communicator.GetProperties();
                    properties.Remove("Ice.Default.Locator");
                    properties["IceLocatorDiscovery.RetryCount"] = "1";
                    properties["Ice.Plugin.IceLocatorDiscovery"] = "Ice:ZeroC.IceLocatorDiscovery.PluginFactory";
                    {
                        string intf = communicator.GetProperty("IceLocatorDiscovery.Interface") ?? "";
                        if (intf.Length > 0)
                        {
                            intf = $" --interface \"{intf}\"";
                        }
                        string port = helper.GetTestPort(99).ToString();
                        properties["IceLocatorDiscovery.Lookup"] =
                            $"udp -h {multicast} --interface unknown:udp -h {multicast} -p {port}{intf}";
                    }
                    using var com = new Communicator(properties);
                    TestHelper.Assert(com.DefaultLocator != null);
                    try
                    {
                        IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                    }
                    catch (NoEndpointException)
                    {
                        TestHelper.Assert(false);
                    }
                }
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("shutting down server... ");
            Console.Out.Flush();
            obj.Shutdown();
            Console.Out.WriteLine("ok");
        }

        public static void RunWithDeploy(TestHelper helper)
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

            var registry = IRegistryPrx.Parse(
                $"{communicator.DefaultLocator!.Identity.Category}/Registry", communicator);
            IAdminSessionPrx? session = registry.CreateAdminSession("foo", "bar");
            TestHelper.Assert(session != null);
            Connection? connection = session.GetConnection()!;
            connection.Acm = new Acm(TimeSpan.FromSeconds(registry.GetACMTimeout()),
                                     connection.Acm.Close,
                                     AcmHeartbeat.Always);

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
