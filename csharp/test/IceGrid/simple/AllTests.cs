//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

public class AllTests : Test.AllTests
{
    public static void allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        String rf = "test @ TestAdapter";
        Ice.ObjectPrx @base = communicator.stringToProxy(rf);
        test(@base != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing IceGrid.Locator is present... ");
        IceGrid.LocatorPrx locator = IceGrid.LocatorPrxHelper.uncheckedCast(@base);
        test(locator != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        test(obj.Equals(@base));
        Console.Out.WriteLine("ok");

        Console.Out.Write("pinging server... ");
        Console.Out.Flush();
        obj.ice_ping();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing locator finder... ");
        Ice.Identity finderId = new Ice.Identity();
        finderId.category = "Ice";
        finderId.name = "LocatorFinder";
        Ice.LocatorFinderPrx finder = Ice.LocatorFinderPrxHelper.checkedCast(
            communicator.getDefaultLocator().ice_identity(finderId));
        test(finder.getLocator() != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing discovery... ");
        {
            // Add test well-known object
            IceGrid.RegistryPrx registry = IceGrid.RegistryPrxHelper.checkedCast(
                communicator.stringToProxy(communicator.getDefaultLocator().ice_getIdentity().category + "/Registry"));
            test(registry != null);

            try
            {
                IceGrid.AdminSessionPrx session = registry.createAdminSession("foo", "bar");
                session.getAdmin().addObjectWithType(@base, "::Test");
                session.destroy();
            }
            catch(Ice.UserException)
            {
                test(false);
            }

            //
            // Ensure the IceGrid discovery locator can discover the
            // registries and make sure locator requests are forwarded.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("Ice.Plugin.IceLocatorDiscovery",
                                            "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory");
            initData.properties.setProperty("IceLocatorDiscovery.Port", helper.getTestPort(99).ToString());
            initData.properties.setProperty("AdapterForDiscoveryTest.AdapterId", "discoveryAdapter");
            initData.properties.setProperty("AdapterForDiscoveryTest.Endpoints", "default");

            Ice.Communicator com =  Ice.Util.initialize(initData);
            test(com.getDefaultLocator() != null);
            com.stringToProxy("test @ TestAdapter").ice_ping();
            com.stringToProxy("test").ice_ping();

            test(com.getDefaultLocator().getRegistry() != null);
            test(IceGrid.LocatorPrxHelper.uncheckedCast(com.getDefaultLocator()).getLocalRegistry() != null);
            test(IceGrid.LocatorPrxHelper.uncheckedCast(com.getDefaultLocator()).getLocalQuery() != null);

            Ice.ObjectAdapter adapter = com.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.activate();
            adapter.deactivate();
            com.destroy();

            //
            // Now, ensure that the IceGrid discovery locator correctly
            // handles failure to find a locator.
            //
            initData.properties.setProperty("IceLocatorDiscovery.InstanceName", "unknown");
            initData.properties.setProperty("IceLocatorDiscovery.RetryCount", "1");
            initData.properties.setProperty("IceLocatorDiscovery.Timeout", "100");
            com = Ice.Util.initialize(initData);
            test(com.getDefaultLocator() != null);
            try
            {
                com.stringToProxy("test @ TestAdapter").ice_ping();
            }
            catch(Ice.NoEndpointException)
            {
            }
            try
            {
                com.stringToProxy("test").ice_ping();
            }
            catch(Ice.NoEndpointException)
            {
            }
            test(com.getDefaultLocator().getRegistry() == null);
            test(IceGrid.LocatorPrxHelper.checkedCast(com.getDefaultLocator()) == null);
            try
            {
                IceGrid.LocatorPrxHelper.uncheckedCast(com.getDefaultLocator()).getLocalRegistry();
            }
            catch(Ice.OperationNotExistException)
            {
            }

            adapter = com.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.activate();
            adapter.deactivate();

            com.destroy();

            string multicast;
            if(communicator.getProperties().getProperty("Ice.IPv6").Equals("1"))
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
            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("Ice.Plugin.IceLocatorDiscovery",
                                            "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory");
            initData.properties.setProperty("IceLocatorDiscovery.Lookup",
                                             "udp -h " + multicast + " --interface unknown");
            com = Ice.Util.initialize(initData);
            test(com.getDefaultLocator() != null);
            try
            {
                com.stringToProxy("test @ TestAdapter").ice_ping();
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
            }
            com.destroy();

            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("IceLocatorDiscovery.RetryCount", "0");
            initData.properties.setProperty("Ice.Plugin.IceLocatorDiscovery",
                                            "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory");
            initData.properties.setProperty("IceLocatorDiscovery.Lookup",
                                             "udp -h " + multicast + " --interface unknown");
            com = Ice.Util.initialize(initData);
            test(com.getDefaultLocator() != null);
            try
            {
                com.stringToProxy("test @ TestAdapter").ice_ping();
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
            }
            com.destroy();

            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("IceLocatorDiscovery.RetryCount", "1");
            initData.properties.setProperty("Ice.Plugin.IceLocatorDiscovery",
                                            "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory");
            {
                string intf = initData.properties.getProperty("IceLocatorDiscovery.Interface");
                if(!intf.Equals(""))
                {
                    intf = " --interface \"" + intf + "\"";
                }
                string port = helper.getTestPort(99).ToString();
                initData.properties.setProperty("IceLocatorDiscovery.Lookup",
                                                 "udp -h " + multicast + " --interface unknown:" +
                                                 "udp -h " + multicast + " -p " + port + intf);
            }
            com = Ice.Util.initialize(initData);
            test(com.getDefaultLocator() != null);
            try
            {
                com.stringToProxy("test @ TestAdapter").ice_ping();
            }
            catch(Ice.NoEndpointException)
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
        Ice.ObjectPrx @base = communicator.stringToProxy("test @ TestAdapter");
        test(@base != null);
        Ice.ObjectPrx @base2 = communicator.stringToProxy("test");
        test(@base2 != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        test(obj.Equals(@base));
        TestIntfPrx obj2 = TestIntfPrxHelper.checkedCast(@base2);
        test(obj2 != null);
        test(obj2.Equals(@base2));
        Console.Out.WriteLine("ok");

        Console.Out.Write("pinging server... ");
        Console.Out.Flush();
        obj.ice_ping();
        obj2.ice_ping();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing encoding versioning... ");
        Console.Out.Flush();
        Ice.ObjectPrx base10 = communicator.stringToProxy("test10 @ TestAdapter10");
        test(base10 != null);
        Ice.ObjectPrx base102 = communicator.stringToProxy("test10");
        test(base102 != null);
        try
        {
            base10.ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException)
        {
        }
        try
        {
            base102.ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException)
        {
        }
        base10 = base10.ice_encodingVersion(Ice.Util.Encoding_1_0);
        base102 = base102.ice_encodingVersion(Ice.Util.Encoding_1_0);
        base10.ice_ping();
        base102.ice_ping();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing reference with unknown identity... ");
        Console.Out.Flush();
        try
        {
            communicator.stringToProxy("unknown/unknown").ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object"));
            test(ex.id.Equals("unknown/unknown"));
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing reference with unknown adapter... ");
        Console.Out.Flush();
        try
        {
            communicator.stringToProxy("test @ TestAdapterUnknown").ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object adapter"));
            test(ex.id.Equals("TestAdapterUnknown"));
        }
        Console.Out.WriteLine("ok");

        IceGrid.RegistryPrx registry = IceGrid.RegistryPrxHelper.checkedCast(
            communicator.stringToProxy(communicator.getDefaultLocator().ice_getIdentity().category + "/Registry"));
        test(registry != null);
        IceGrid.AdminSessionPrx session = null;
        try
        {
            session = registry.createAdminSession("foo", "bar");
        }
        catch(IceGrid.PermissionDeniedException)
        {
            test(false);
        }

        session.ice_getConnection().setACM(registry.getACMTimeout(),
                                           Ice.Util.None,
                                           Ice.ACMHeartbeat.HeartbeatAlways);

        IceGrid.AdminPrx admin = session.getAdmin();
        test(admin != null);

        try
        {
            admin.enableServer("server", false);
            admin.stopServer("server");
        }
        catch(IceGrid.ServerNotExistException)
        {
            test(false);
        }
        catch(IceGrid.ServerStopException)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException)
        {
            test(false);
        }

        Console.Out.Write("testing whether server is still reachable... ");
        Console.Out.Flush();
        try
        {
            obj = TestIntfPrxHelper.checkedCast(@base);
            test(false);
        }
        catch(Ice.NoEndpointException)
        {
        }
        try
        {
            obj2 = TestIntfPrxHelper.checkedCast(@base2);
            test(false);
        }
        catch(Ice.NoEndpointException)
        {
        }

        try
        {
            admin.enableServer("server", true);
        }
        catch(IceGrid.ServerNotExistException)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException)
        {
            test(false);
        }

        try
        {
            obj = TestIntfPrxHelper.checkedCast(@base);
        }
        catch(Ice.NoEndpointException)
        {
            test(false);
        }
        try
        {
            obj2 = TestIntfPrxHelper.checkedCast(@base2);
        }
        catch(Ice.NoEndpointException)
        {
            test(false);
        }
        Console.Out.WriteLine("ok");

        try
        {
            admin.stopServer("server");
        }
        catch(IceGrid.ServerNotExistException)
        {
            test(false);
        }
        catch(IceGrid.ServerStopException)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException)
        {
            test(false);
        }

        session.destroy();
    }
}
