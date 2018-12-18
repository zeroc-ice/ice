// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceGrid.simple;
import java.io.PrintWriter;

import test.IceGrid.simple.Test.TestIntfPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.TestHelper helper)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test @ TestAdapter";
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing IceGrid.Locator is present... ");
        com.zeroc.IceGrid.LocatorPrx locator = com.zeroc.IceGrid.LocatorPrx.uncheckedCast(base);
        test(locator != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrx.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        out.print("pinging server... ");
        out.flush();
        obj.ice_ping();
        out.println("ok");

        out.print("testing locator finder... ");
        com.zeroc.Ice.Identity finderId = new com.zeroc.Ice.Identity();
        finderId.category = "Ice";
        finderId.name = "LocatorFinder";
        com.zeroc.Ice.LocatorFinderPrx finder = com.zeroc.Ice.LocatorFinderPrx.checkedCast(
            communicator.getDefaultLocator().ice_identity(finderId));
        test(finder.getLocator() != null);
        out.println("ok");

        out.print("testing discovery... ");
        out.flush();
        {
            // Add test well-known object
            com.zeroc.IceGrid.RegistryPrx registry = com.zeroc.IceGrid.RegistryPrx.checkedCast(
                communicator.stringToProxy(communicator.getDefaultLocator().ice_getIdentity().category + "/Registry"));
            test(registry != null);

            try
            {
                com.zeroc.IceGrid.AdminSessionPrx session = registry.createAdminSession("foo", "bar");
                session.getAdmin().addObjectWithType(base, "::Test");
                session.destroy();
            }
            catch(com.zeroc.Ice.UserException ex)
            {
                test(false);
            }

            //
            // Ensure the IceGrid discovery locator can discover the
            // registries and make sure locator requests are forwarded.
            //
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("Ice.Plugin.IceLocatorDiscovery",
                                            "IceLocatorDiscovery:com.zeroc.IceLocatorDiscovery.PluginFactory");
            initData.properties.setProperty("IceLocatorDiscovery.Port", Integer.toString(helper.getTestPort(99)));
            initData.properties.setProperty("AdapterForDiscoveryTest.AdapterId", "discoveryAdapter");
            initData.properties.setProperty("AdapterForDiscoveryTest.Endpoints", "default");

            com.zeroc.Ice.Communicator comm = com.zeroc.Ice.Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            comm.stringToProxy("test @ TestAdapter").ice_ping();
            comm.stringToProxy("test").ice_ping();

            test(comm.getDefaultLocator().getRegistry() != null);
            test(com.zeroc.IceGrid.LocatorPrx.uncheckedCast(comm.getDefaultLocator()).getLocalRegistry() != null);
            test(com.zeroc.IceGrid.LocatorPrx.uncheckedCast(comm.getDefaultLocator()).getLocalQuery() != null);

            com.zeroc.Ice.ObjectAdapter adapter = comm.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.activate();
            adapter.deactivate();
            comm.destroy();

            //
            // Now, ensure that the IceGrid discovery locator correctly
            // handles failure to find a locator.
            //
            initData.properties.setProperty("IceLocatorDiscovery.InstanceName", "unknown");
            initData.properties.setProperty("IceLocatorDiscovery.RetryCount", "1");
            initData.properties.setProperty("IceLocatorDiscovery.Timeout", "100");
            comm = com.zeroc.Ice.Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            try
            {
                comm.stringToProxy("test @ TestAdapter").ice_ping();
            }
            catch(com.zeroc.Ice.NoEndpointException ex)
            {
            }
            try
            {
                comm.stringToProxy("test").ice_ping();
            }
            catch(com.zeroc.Ice.NoEndpointException ex)
            {
            }
            test(comm.getDefaultLocator().getRegistry() == null);
            test(com.zeroc.IceGrid.LocatorPrx.checkedCast(comm.getDefaultLocator()) == null);
            try
            {
                com.zeroc.IceGrid.LocatorPrx.uncheckedCast(comm.getDefaultLocator()).getLocalQuery();
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
            }

            adapter = comm.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.activate();
            adapter.deactivate();

            comm.destroy();

            String multicast;
            if(communicator.getProperties().getProperty("Ice.IPv6").equals("1"))
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
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("Ice.Plugin.IceLocatorDiscovery",
                                            "com.zeroc.IceLocatorDiscovery.PluginFactory");
            initData.properties.setProperty("IceLocatorDiscovery.Lookup",
                                             "udp -h " + multicast + " --interface unknown");
            comm = com.zeroc.Ice.Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            try
            {
                comm.stringToProxy("test @ TestAdapter").ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.NoEndpointException ex)
            {
            }
            comm.destroy();

            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("Ice.Plugin.IceLocatorDiscovery",
                                            "com.zeroc.IceLocatorDiscovery.PluginFactory");
            {
                String intf = initData.properties.getProperty("IceLocatorDiscovery.Interface");
                if(!intf.isEmpty())
                {
                    intf = " --interface \"" + intf + "\"";
                }
                String port = Integer.toString(helper.getTestPort(99));
                initData.properties.setProperty("IceLocatorDiscovery.Lookup",
                                                 "udp -h " + multicast + " --interface unknown:" +
                                                 "udp -h " + multicast + " -p " + port + intf);
            }
            comm = com.zeroc.Ice.Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            try
            {
                comm.stringToProxy("test @ TestAdapter").ice_ping();
            }
            catch(com.zeroc.Ice.NoEndpointException ex)
            {
                test(false);
            }
            comm.destroy();
        }
        out.println("ok");

        out.print("shutting down server... ");
        out.flush();
        obj.shutdown();
        out.println("ok");
    }

    public static void allTestsWithDeploy(test.TestHelper helper)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
        test(base != null);
        com.zeroc.Ice.ObjectPrx base2 = communicator.stringToProxy("test");
        test(base2 != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrx.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        TestIntfPrx obj2 = TestIntfPrx.checkedCast(base2);
        test(obj2 != null);
        test(obj2.equals(base2));
        out.println("ok");

        out.print("pinging server... ");
        out.flush();
        obj.ice_ping();
        obj2.ice_ping();
        out.println("ok");

        out.print("testing encoding versioning... ");
        out.flush();
        com.zeroc.Ice.ObjectPrx base10 = communicator.stringToProxy("test10 @ TestAdapter10");
        test(base10 != null);
        com.zeroc.Ice.ObjectPrx base102 = communicator.stringToProxy("test10");
        test(base102 != null);
        try
        {
            base10.ice_ping();
            test(false);
        }
        catch(com.zeroc.Ice.NoEndpointException ex)
        {
        }
        try
        {
            base102.ice_ping();
            test(false);
        }
        catch(com.zeroc.Ice.NoEndpointException ex)
        {
        }
        base10 = base10.ice_encodingVersion(com.zeroc.Ice.Util.Encoding_1_0);
        base102 = base102.ice_encodingVersion(com.zeroc.Ice.Util.Encoding_1_0);
        base10.ice_ping();
        base102.ice_ping();
        out.println("ok");

        out.print("testing reference with unknown identity... ");
        out.flush();
        try
        {
            communicator.stringToProxy("unknown/unknown").ice_ping();
            test(false);
        }
        catch(com.zeroc.Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.equals("object"));
            test(ex.id.equals("unknown/unknown"));
        }
        out.println("ok");

        out.print("testing reference with unknown adapter... ");
        out.flush();
        try
        {
            communicator.stringToProxy("test @ TestAdapterUnknown").ice_ping();
            test(false);
        }
        catch(com.zeroc.Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.equals("object adapter"));
            test(ex.id.equals("TestAdapterUnknown"));
        }
        out.println("ok");

        com.zeroc.IceGrid.RegistryPrx registry = com.zeroc.IceGrid.RegistryPrx.checkedCast(
            communicator.stringToProxy(communicator.getDefaultLocator().ice_getIdentity().category + "/Registry"));
        test(registry != null);
        com.zeroc.IceGrid.AdminSessionPrx session = null;
        try
        {
            session = registry.createAdminSession("foo", "bar");
        }
        catch(com.zeroc.IceGrid.PermissionDeniedException e)
        {
            test(false);
        }

        session.ice_getConnection().setACM(java.util.OptionalInt.of(registry.getACMTimeout()), null,
                                           java.util.Optional.of(com.zeroc.Ice.ACMHeartbeat.HeartbeatAlways));

        com.zeroc.IceGrid.AdminPrx admin = session.getAdmin();
        test(admin != null);

        try
        {
            admin.enableServer("server", false);
            admin.stopServer("server");
        }
        catch(com.zeroc.IceGrid.ServerNotExistException ex)
        {
            test(false);
        }
        catch(com.zeroc.IceGrid.ServerStopException ex)
        {
            test(false);
        }
        catch(com.zeroc.IceGrid.NodeUnreachableException ex)
        {
            test(false);
        }
        catch(com.zeroc.IceGrid.DeploymentException ex)
        {
            test(false);
        }

        out.print("testing whether server is still reachable... ");
        out.flush();
        try
        {
            obj = TestIntfPrx.checkedCast(base);
            test(false);
        }
        catch(com.zeroc.Ice.NoEndpointException ex)
        {
        }
        try
        {
            obj2 = TestIntfPrx.checkedCast(base2);
            test(false);
        }
        catch(com.zeroc.Ice.NoEndpointException ex)
        {
        }

        try
        {
            admin.enableServer("server", true);
        }
        catch(com.zeroc.IceGrid.ServerNotExistException ex)
        {
            test(false);
        }
        catch(com.zeroc.IceGrid.NodeUnreachableException ex)
        {
            test(false);
        }
        catch(com.zeroc.IceGrid.DeploymentException ex)
        {
            test(false);
        }

        try
        {
            obj = TestIntfPrx.checkedCast(base);
        }
        catch(com.zeroc.Ice.NoEndpointException ex)
        {
            test(false);
        }
        try
        {
            obj2 = TestIntfPrx.checkedCast(base2);
        }
        catch(com.zeroc.Ice.NoEndpointException ex)
        {
            test(false);
        }
        out.println("ok");

        try
        {
            admin.stopServer("server");
        }
        catch(com.zeroc.IceGrid.ServerNotExistException ex)
        {
            test(false);
        }
        catch(com.zeroc.IceGrid.ServerStopException ex)
        {
            test(false);
        }
        catch(com.zeroc.IceGrid.NodeUnreachableException ex)
        {
            test(false);
        }
        catch(com.zeroc.IceGrid.DeploymentException ex)
        {
            test(false);
        }

        session.destroy();
    }
}
