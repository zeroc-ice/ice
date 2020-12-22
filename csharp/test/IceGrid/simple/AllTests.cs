// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.IceGrid.Test.Simple
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator;
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
            ILocatorFinderPrx finder = communicator.DefaultLocator!.Clone(ILocatorFinderPrx.Factory,
                                                                          identity: finderId);
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

                // Ensure locator discovery can discover the registries and make sure locator requests are forwarded.
                Dictionary<string, string>? properties = communicator.GetProperties();
                properties["Ice.Default.Locator"] = "LocatorDiscovery";
                properties["AdapterForDiscoveryTest.AdapterId"] = "discoveryAdapter";
                properties["AdapterForDiscoveryTest.Endpoints"] = $"{helper.Transport} -h 127.0.0.1";

                {
                    using var com = new Communicator(properties);
                    TestHelper.Assert(com.DefaultLocator != null);
                    IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                    IObjectPrx.Parse("test", com).IcePing();

                    Ice.ILocatorPrx defaultLocator = com.DefaultLocator!;
                    TestHelper.Assert(defaultLocator.Protocol == Protocol.Ice2);
                    TestHelper.Assert(defaultLocator.Encoding == Encoding.V20);

                    // This works fine because the LocatorDiscovery Locator performs transcoding for Ice::Locator
                    // operations.
                    TestHelper.Assert(defaultLocator.GetRegistry() != null);

                    // CheckedCast on the _IceGrid_ Locator proxy fails because ice_isA is not forwarded (due to the
                    // encoding mismatch) but is instead implemented by the plain Locator of LocatorDiscovery.
                    TestHelper.Assert(defaultLocator.CheckedCast(ILocatorPrx.Factory) == null);

                    // Change the encoding to make it work:
                    defaultLocator = defaultLocator.Clone(encoding: Encoding.V11);
                    var iceGridLocator = defaultLocator.CheckedCast(ILocatorPrx.Factory);
                    TestHelper.Assert(iceGridLocator != null);
                    TestHelper.Assert(iceGridLocator.GetLocalRegistry() != null);
                    TestHelper.Assert(iceGridLocator.GetLocalQuery() != null);

                    ObjectAdapter adapter = com.CreateObjectAdapter("AdapterForDiscoveryTest");
                    await adapter.ActivateAsync();
                }
                // Now, ensure that locator discovery correctly handles failure to find a locator.
                {
                    properties["Ice.LocatorDiscovery.InstanceName"] = "unknown";
                    properties["Ice.LocatorDiscovery.RetryCount"] = "1";
                    properties["Ice.LocatorDiscovery.Timeout"] = "100ms";
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

                    try
                    {
                        IObjectPrx.Parse("test", com).IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (NoEndpointException)
                    {
                    }

                    Ice.ILocatorPrx defaultLocator = com.DefaultLocator!.Clone(encoding: Encoding.V11);

                    try
                    {
                        defaultLocator.Clone(ILocatorPrx.Factory).GetLocalRegistry();
                        TestHelper.Assert(false);
                    }
                    catch (OperationNotExistException)
                    {
                    }

                    await using ObjectAdapter adapter = com.CreateObjectAdapter("AdapterForDiscoveryTest");
                    await adapter.ActivateAsync();
                }

                string multicast;
                if (helper.Host.Contains(":"))
                {
                    multicast = "\"ff15::1\"";
                }
                else
                {
                    multicast = "239.255.0.1";
                }

                // Test invalid lookup endpoints
                {
                    properties = communicator.GetProperties();
                    properties["Ice.Default.Locator"] = "locatordiscovery";
                    properties["Ice.LocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
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
                    properties["Ice.Default.Locator"] = "locatordiscovery";
                    properties["Ice.LocatorDiscovery.RetryCount"] = "0";
                    properties["Ice.LocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
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
                    properties["Ice.Default.Locator"] = "locatordiscovery";
                    properties["Ice.LocatorDiscovery.RetryCount"] = "1";
                    {
                        string intf = helper.Host.Contains(":") ? $"\"{helper.Host}\"" : helper.Host;
                        string port = $"{helper.BasePort + 99}";
                        properties["Ice.LocatorDiscovery.Lookup"] =
                            $"udp -h {multicast} --interface unknown:udp -h {multicast} -p {port} --interface {intf}";
                    }
                    using var com = new Communicator(properties);
                    TestHelper.Assert(com.DefaultLocator != null);
                    IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
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
            Communicator? communicator = helper.Communicator;
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
            catch (NoEndpointException)
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
            catch (NoEndpointException)
            {
                // expected
            }
            Console.Out.WriteLine("ok");

            var registry = IRegistryPrx.Parse(
                $"{communicator.DefaultLocator!.Identity.Category}/Registry", communicator);
            IAdminSessionPrx? session = registry.CreateAdminSession("foo", "bar");
            TestHelper.Assert(session != null);
            Connection connection = session.GetConnection();
            connection.KeepAlive = true;

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

            admin.EnableServer("server", true);
            obj.IcePing();
            obj2.IcePing();
            Console.Out.WriteLine("ok");

            admin.StopServer("server");
            session.Destroy();
        }
    }
}
