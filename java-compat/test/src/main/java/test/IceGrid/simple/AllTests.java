// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceGrid.simple;
import java.io.PrintWriter;

import test.IceGrid.simple.Test.TestIntfPrx;
import test.IceGrid.simple.Test.TestIntfPrxHelper;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(test.Util.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test @ TestAdapter";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing IceGrid.Locator is present... ");
        IceGrid.LocatorPrx locator = IceGrid.LocatorPrxHelper.uncheckedCast(base);
        test(locator != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        out.print("pinging server... ");
        out.flush();
        obj.ice_ping();
        out.println("ok");

        out.print("testing locator finder... ");
        Ice.Identity finderId = new Ice.Identity();
        finderId.category = "Ice";
        finderId.name = "LocatorFinder";
        Ice.LocatorFinderPrx finder = Ice.LocatorFinderPrxHelper.checkedCast(
            communicator.getDefaultLocator().ice_identity(finderId));
        test(finder.getLocator() != null);
        out.println("ok");

        out.print("testing discovery... ");
        out.flush();
        {
            // Add test well-known object
            IceGrid.RegistryPrx registry = IceGrid.RegistryPrxHelper.checkedCast(
                communicator.stringToProxy(communicator.getDefaultLocator().ice_getIdentity().category + "/Registry"));
            test(registry != null);

            try
            {
                IceGrid.AdminSessionPrx session = registry.createAdminSession("foo", "bar");
                session.getAdmin().addObjectWithType(base, "::Test");
                session.destroy();
            }
            catch(Ice.UserException ex)
            {
                test(false);
            }

            //
            // Ensure the IceGrid discovery locator can discover the
            // registries and make sure locator requests are forwarded.
            //
            Ice.InitializationData initData = app.createInitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("Ice.Plugin.IceLocatorDiscovery",
                                            "IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory");
            if(System.getProperty("os.name").contains("OS X") &&
               initData.properties.getPropertyAsInt("Ice.PreferIPv6Address") > 0)
            {
                initData.properties.setProperty("IceLocatorDiscovery.Interface", "::1");
            }
            initData.properties.setProperty("IceLocatorDiscovery.Port", Integer.toString(app.getTestPort(99)));
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
            catch(Ice.NoEndpointException ex)
            {
            }
            try
            {
                com.stringToProxy("test").ice_ping();
            }
            catch(Ice.NoEndpointException ex)
            {
            }
            test(com.getDefaultLocator().getRegistry() == null);
            test(IceGrid.LocatorPrxHelper.checkedCast(com.getDefaultLocator()) == null);
            try
            {
                IceGrid.LocatorPrxHelper.uncheckedCast(com.getDefaultLocator()).getLocalQuery();
            }
            catch(Ice.OperationNotExistException ex)
            {
            }

            adapter = com.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.activate();
            adapter.deactivate();

            com.destroy();
        }
        out.println("ok");

        out.print("shutting down server... ");
        out.flush();
        obj.shutdown();
        out.println("ok");
    }

    public static void
    allTestsWithDeploy(test.Util.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        Ice.ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
        test(base != null);
        Ice.ObjectPrx base2 = communicator.stringToProxy("test");
        test(base2 != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        TestIntfPrx obj2 = TestIntfPrxHelper.checkedCast(base2);
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
        Ice.ObjectPrx base10 = communicator.stringToProxy("test10 @ TestAdapter10");
        test(base10 != null);
        Ice.ObjectPrx base102 = communicator.stringToProxy("test10");
        test(base102 != null);
        try
        {
            base10.ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException ex)
        {
        }
        try
        {
            base102.ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException ex)
        {
        }
        base10 = base10.ice_encodingVersion(Ice.Util.Encoding_1_0);
        base102 = base102.ice_encodingVersion(Ice.Util.Encoding_1_0);
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
        catch(Ice.NotRegisteredException ex)
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
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.equals("object adapter"));
            test(ex.id.equals("TestAdapterUnknown"));
        }
        out.println("ok");

        IceGrid.RegistryPrx registry = IceGrid.RegistryPrxHelper.checkedCast(
            communicator.stringToProxy(communicator.getDefaultLocator().ice_getIdentity().category + "/Registry"));
        test(registry != null);
        IceGrid.AdminSessionPrx session = null;
        try
        {
            session = registry.createAdminSession("foo", "bar");
        }
        catch(IceGrid.PermissionDeniedException e)
        {
            test(false);
        }

        session.ice_getConnection().setACM(new Ice.IntOptional(registry.getACMTimeout()), null,
                                           new Ice.Optional<Ice.ACMHeartbeat>(Ice.ACMHeartbeat.HeartbeatAlways));

        IceGrid.AdminPrx admin = session.getAdmin();
        test(admin != null);

        try
        {
            admin.enableServer("server", false);
            admin.stopServer("server");
        }
        catch(IceGrid.ServerNotExistException ex)
        {
            test(false);
        }
        catch(IceGrid.ServerStopException ex)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException ex)
        {
            test(false);
        }
        catch(IceGrid.DeploymentException ex)
        {
            test(false);
        }

        out.print("testing whether server is still reachable... ");
        out.flush();
        try
        {
            obj = TestIntfPrxHelper.checkedCast(base);
            test(false);
        }
        catch(Ice.NoEndpointException ex)
        {
        }
        try
        {
            obj2 = TestIntfPrxHelper.checkedCast(base2);
            test(false);
        }
        catch(Ice.NoEndpointException ex)
        {
        }

        try
        {
            admin.enableServer("server", true);
        }
        catch(IceGrid.ServerNotExistException ex)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException ex)
        {
            test(false);
        }
        catch(IceGrid.DeploymentException ex)
        {
            test(false);
        }

        try
        {
            obj = TestIntfPrxHelper.checkedCast(base);
        }
        catch(Ice.NoEndpointException ex)
        {
            test(false);
        }
        try
        {
            obj2 = TestIntfPrxHelper.checkedCast(base2);
        }
        catch(Ice.NoEndpointException ex)
        {
            test(false);
        }
        out.println("ok");

        try
        {
            admin.stopServer("server");
        }
        catch(IceGrid.ServerNotExistException ex)
        {
            test(false);
        }
        catch(IceGrid.ServerStopException ex)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException ex)
        {
            test(false);
        }
        catch(IceGrid.DeploymentException ex)
        {
            test(false);
        }

        session.destroy();
    }
}
