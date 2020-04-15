//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;
using Ice;

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
            ILocatorFinderPrx.CheckedCast(communicator.GetDefaultLocator()!.Clone(finderId, IObjectPrx.Factory));
        TestHelper.Assert(finder != null && finder.GetLocator() != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing discovery... ");
        {
            // Add test well-known object
            IceGrid.IRegistryPrx? registry = IceGrid.IRegistryPrx.Parse(
                communicator.GetDefaultLocator()!.Identity.Category + "/Registry", communicator);

            IceGrid.IAdminSessionPrx? session = registry.CreateAdminSession("foo", "bar");
            TestHelper.Assert(session != null);
            session.GetAdmin()!.AddObjectWithType(obj, "::Test");
            session.Destroy();

            //
            // Ensure the IceGrid discovery locator can discover the
            // registries and make sure locator requests are forwarded.
            //
            var properties = communicator.GetProperties();
            properties.Remove("Ice.Default.Locator");
            properties["Ice.Plugin.IceLocatorDiscovery"] = "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory";
            properties["IceLocatorDiscovery.Port"] = helper.GetTestPort(99).ToString();
            properties["AdapterForDiscoveryTest.AdapterId"] = "discoveryAdapter";
            properties["AdapterForDiscoveryTest.Endpoints"] = "default";

            var com = new Communicator(properties);
            TestHelper.Assert(com.GetDefaultLocator() != null);
            IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
            IObjectPrx.Parse("test", com).IcePing();

            TestHelper.Assert(com.GetDefaultLocator()!.GetRegistry() != null);
            TestHelper.Assert(IceGrid.ILocatorPrx.UncheckedCast(com.GetDefaultLocator()!).GetLocalRegistry() != null);
            TestHelper.Assert(IceGrid.ILocatorPrx.UncheckedCast(com.GetDefaultLocator()!).GetLocalQuery() != null);

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
            TestHelper.Assert(com.GetDefaultLocator() != null);
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
            TestHelper.Assert(com.GetDefaultLocator()!.GetRegistry() == null);
            TestHelper.Assert(IceGrid.ILocatorPrx.CheckedCast(com.GetDefaultLocator()!) == null);
            try
            {
                IceGrid.ILocatorPrx.UncheckedCast(com.GetDefaultLocator()!).GetLocalRegistry();
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
            properties["Ice.Plugin.IceLocatorDiscovery"] = "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory";
            properties["IceLocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
            com = new Communicator(properties);
            TestHelper.Assert(com.GetDefaultLocator() != null);
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
            properties["Ice.Plugin.IceLocatorDiscovery"] = "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory";
            properties["IceLocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
            com = new Communicator(properties);
            TestHelper.Assert(com.GetDefaultLocator() != null);
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
            properties["Ice.Plugin.IceLocatorDiscovery"] = "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory";
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
            TestHelper.Assert(com.GetDefaultLocator() != null);
            try
            {
                IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
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

        IceGrid.IRegistryPrx registry = IceGrid.IRegistryPrx.Parse(
            communicator.GetDefaultLocator()!.Identity.Category + "/Registry", communicator);
        IceGrid.IAdminSessionPrx? session = registry.CreateAdminSession("foo", "bar");
        TestHelper.Assert(session != null);
        session.GetConnection().SetACM(registry.GetACMTimeout(), null, ACMHeartbeat.HeartbeatAlways);

        IceGrid.IAdminPrx? admin = session.GetAdmin();
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
        catch (Ice.NoEndpointException)
        {
        }
        try
        {
            obj2.IcePing();
            TestHelper.Assert(false);
        }
        catch (Ice.NoEndpointException)
        {
        }

        try
        {
            admin.EnableServer("server", true);
        }
        catch (IceGrid.ServerNotExistException)
        {
            TestHelper.Assert(false);
        }
        catch (IceGrid.NodeUnreachableException)
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
        catch (Ice.NoEndpointException)
        {
            TestHelper.Assert(false);
        }
        Console.Out.WriteLine("ok");

        try
        {
            admin.StopServer("server");
        }
        catch (IceGrid.ServerNotExistException)
        {
            TestHelper.Assert(false);
        }
        catch (IceGrid.ServerStopException)
        {
            TestHelper.Assert(false);
        }
        catch (IceGrid.NodeUnreachableException)
        {
            TestHelper.Assert(false);
        }

        session.Destroy();
    }
}
