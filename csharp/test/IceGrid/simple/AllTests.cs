//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;
using Ice;

public class AllTests : Test.AllTests
{
    public static void allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string rf = "test @ TestAdapter";
        var @base = IObjectPrx.Parse(rf, communicator);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing IceGrid.Locator is present... ");
        IceGrid.LocatorPrx locator = IceGrid.LocatorPrx.UncheckedCast(@base);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrx.CheckedCast(@base);
        test(obj.Equals(@base));
        Console.Out.WriteLine("ok");

        Console.Out.Write("pinging server... ");
        Console.Out.Flush();
        obj.IcePing();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing locator finder... ");
        Ice.Identity finderId = new Ice.Identity();
        finderId.category = "Ice";
        finderId.name = "LocatorFinder";
        Ice.LocatorFinderPrx finder = Ice.LocatorFinderPrx.CheckedCast(communicator.getDefaultLocator().Clone(finderId));
        test(finder.getLocator() != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing discovery... ");
        {
            // Add test well-known object
            IceGrid.RegistryPrx registry = IceGrid.RegistryPrx.Parse(
                communicator.getDefaultLocator().Identity.category + "/Registry", communicator);

            IceGrid.AdminSessionPrx session = registry.createAdminSession("foo", "bar");
            session.getAdmin().addObjectWithType(@base, "::Test");
            session.destroy();

            //
            // Ensure the IceGrid discovery locator can discover the
            // registries and make sure locator requests are forwarded.
            //
            var properties = communicator.GetProperties();
            properties.Remove("Ice.Default.Locator");
            properties["Ice.Plugin.IceLocatorDiscovery"] = "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory";
            properties["IceLocatorDiscovery.Port"] = helper.getTestPort(99).ToString();
            properties["AdapterForDiscoveryTest.AdapterId"] = "discoveryAdapter";
            properties["AdapterForDiscoveryTest.Endpoints"] = "default";

            Communicator com = new Communicator(properties);
            test(com.getDefaultLocator() != null);
            IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
            IObjectPrx.Parse("test", com).IcePing();

            test(com.getDefaultLocator().getRegistry() != null);
            test(IceGrid.LocatorPrx.UncheckedCast(com.getDefaultLocator()).getLocalRegistry() != null);
            test(IceGrid.LocatorPrx.UncheckedCast(com.getDefaultLocator()).getLocalQuery() != null);

            Ice.ObjectAdapter adapter = com.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.Activate();
            adapter.Deactivate();
            com.destroy();

            //
            // Now, ensure that the IceGrid discovery locator correctly
            // handles failure to find a locator.
            //
            properties["IceLocatorDiscovery.InstanceName"] = "unknown";
            properties["IceLocatorDiscovery.RetryCount"] = "1";
            properties["IceLocatorDiscovery.Timeout"] = "100";
            com = new Communicator(properties);
            test(com.getDefaultLocator() != null);
            try
            {
                IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
            }
            catch (Ice.NoEndpointException)
            {
            }
            try
            {
                IObjectPrx.Parse("test", com).IcePing();
            }
            catch (Ice.NoEndpointException)
            {
            }
            test(com.getDefaultLocator().getRegistry() == null);
            test(IceGrid.LocatorPrx.CheckedCast(com.getDefaultLocator()) == null);
            try
            {
                IceGrid.LocatorPrx.UncheckedCast(com.getDefaultLocator()).getLocalRegistry();
            }
            catch (Ice.OperationNotExistException)
            {
            }

            adapter = com.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.Activate();
            adapter.Deactivate();

            com.destroy();

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
            test(com.getDefaultLocator() != null);
            try
            {
                IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                test(false);
            }
            catch (NoEndpointException)
            {
            }
            com.destroy();

            properties = communicator.GetProperties();
            properties.Remove("Ice.Default.Locator");
            properties["IceLocatorDiscovery.RetryCount"] = "0";
            properties["Ice.Plugin.IceLocatorDiscovery"] = "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory";
            properties["IceLocatorDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
            com = new Communicator(properties);
            test(com.getDefaultLocator() != null);
            try
            {
                IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
                test(false);
            }
            catch (NoEndpointException)
            {
            }
            com.destroy();

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
                string port = helper.getTestPort(99).ToString();
                properties["IceLocatorDiscovery.Lookup"] =
                    $"udp -h {multicast} --interface unknown:udp -h {multicast} -p {port}{intf}";
            }
            com = new Communicator(properties);
            test(com.getDefaultLocator() != null);
            try
            {
                IObjectPrx.Parse("test @ TestAdapter", com).IcePing();
            }
            catch (NoEndpointException)
            {
                test(false);
            }
            com.destroy();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("shutting down server... ");
        Console.Out.Flush();
        obj.shutdown();
        Console.Out.WriteLine("ok");
    }

    public static void
    allTestsWithDeploy(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        IObjectPrx @base = IObjectPrx.Parse("test @ TestAdapter", communicator);
        test(@base != null);
        IObjectPrx @base2 = IObjectPrx.Parse("test", communicator);
        test(@base2 != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrx.CheckedCast(@base);
        test(obj.Equals(@base));
        TestIntfPrx obj2 = TestIntfPrx.CheckedCast(@base2);
        test(obj2.Equals(@base2));
        Console.Out.WriteLine("ok");

        Console.Out.Write("pinging server... ");
        Console.Out.Flush();
        obj.IcePing();
        obj2.IcePing();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing encoding versioning... ");
        Console.Out.Flush();
        IObjectPrx base10 = IObjectPrx.Parse("test10 @ TestAdapter10", communicator);
        IObjectPrx base102 = IObjectPrx.Parse("test10", communicator);
        try
        {
            base10.IcePing();
            test(false);
        }
        catch (Ice.NoEndpointException)
        {
        }
        try
        {
            base102.IcePing();
            test(false);
        }
        catch (Ice.NoEndpointException)
        {
        }
        base10 = base10.Clone(encodingVersion: Util.Encoding_1_0);
        base102 = base102.Clone(encodingVersion: Util.Encoding_1_0);
        base10.IcePing();
        base102.IcePing();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing reference with unknown identity... ");
        Console.Out.Flush();
        try
        {
            IObjectPrx.Parse("unknown/unknown", communicator).IcePing();
            test(false);
        }
        catch (NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object"));
            test(ex.id.Equals("unknown/unknown"));
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing reference with unknown adapter... ");
        Console.Out.Flush();
        try
        {
            IObjectPrx.Parse("test @ TestAdapterUnknown", communicator).IcePing();
            test(false);
        }
        catch (NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object adapter"));
            test(ex.id.Equals("TestAdapterUnknown"));
        }
        Console.Out.WriteLine("ok");

        IceGrid.RegistryPrx registry = IceGrid.RegistryPrx.Parse(
            communicator.getDefaultLocator().Identity.category + "/Registry", communicator);
        IceGrid.AdminSessionPrx session = registry.createAdminSession("foo", "bar");
        session.GetConnection().setACM(registry.getACMTimeout(), null, ACMHeartbeat.HeartbeatAlways);

        IceGrid.AdminPrx admin = session.getAdmin();
        admin.enableServer("server", false);
        admin.stopServer("server");

        Console.Out.Write("testing whether server is still reachable... ");
        Console.Out.Flush();
        try
        {
            obj = TestIntfPrx.CheckedCast(@base);
            test(false);
        }
        catch (Ice.NoEndpointException)
        {
        }
        try
        {
            obj2 = TestIntfPrx.CheckedCast(@base2);
            test(false);
        }
        catch (Ice.NoEndpointException)
        {
        }

        try
        {
            admin.enableServer("server", true);
        }
        catch (IceGrid.ServerNotExistException)
        {
            test(false);
        }
        catch (IceGrid.NodeUnreachableException)
        {
            test(false);
        }

        try
        {
            obj = TestIntfPrx.CheckedCast(@base);
        }
        catch (Ice.NoEndpointException)
        {
            test(false);
        }
        try
        {
            obj2 = TestIntfPrx.CheckedCast(@base2);
        }
        catch (Ice.NoEndpointException)
        {
            test(false);
        }
        Console.Out.WriteLine("ok");

        try
        {
            admin.stopServer("server");
        }
        catch (IceGrid.ServerNotExistException)
        {
            test(false);
        }
        catch (IceGrid.ServerStopException)
        {
            test(false);
        }
        catch (IceGrid.NodeUnreachableException)
        {
            test(false);
        }

        session.destroy();
    }
}
