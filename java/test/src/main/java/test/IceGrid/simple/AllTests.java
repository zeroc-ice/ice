// Copyright (c) ZeroC, Inc.

package test.IceGrid.simple;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.LocatorFinderPrx;
import com.zeroc.Ice.NoEndpointException;
import com.zeroc.Ice.NotRegisteredException;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationNotExistException;
import com.zeroc.Ice.UserException;
import com.zeroc.Ice.Util;
import com.zeroc.IceGrid.AdminPrx;
import com.zeroc.IceGrid.AdminSessionPrx;
import com.zeroc.IceGrid.DeploymentException;
import com.zeroc.IceGrid.LocatorPrx;
import com.zeroc.IceGrid.NodeUnreachableException;
import com.zeroc.IceGrid.PermissionDeniedException;
import com.zeroc.IceGrid.RegistryPrx;
import com.zeroc.IceGrid.ServerNotExistException;
import com.zeroc.IceGrid.ServerStopException;
import com.zeroc.IceLocatorDiscovery.PluginFactory;

import test.IceGrid.simple.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.Collections;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test @ TestAdapter";
        ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing IceGrid.Locator is present... ");
        LocatorPrx locator = LocatorPrx.uncheckedCast(base);
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
        Identity finderId = new Identity();
        finderId.category = "Ice";
        finderId.name = "LocatorFinder";
        LocatorFinderPrx finder =
            LocatorFinderPrx.checkedCast(
                communicator.getDefaultLocator().ice_identity(finderId));
        test(finder.getLocator() != null);
        out.println("ok");

        out.print("testing discovery... ");
        out.flush();
        {
            // Add test well-known object
            RegistryPrx registry =
                RegistryPrx.checkedCast(
                    communicator.stringToProxy(
                        communicator.getDefaultLocator().ice_getIdentity().category
                            + "/Registry"));
            test(registry != null);

            try {
                AdminSessionPrx session =
                    registry.createAdminSession("foo", "bar");
                session.getAdmin().addObjectWithType(base, "::Test");
                session.destroy();
            } catch (UserException ex) {
                test(false);
            }

            // Ensure the IceGrid discovery locator can discover the registries and make sure
            // locator requests are forwarded.
            InitializationData initData = new InitializationData();
            initData.pluginFactories = Collections.singletonList(new PluginFactory());

            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty(
                "IceLocatorDiscovery.Port", Integer.toString(helper.getTestPort(99)));
            initData.properties.setProperty(
                "AdapterForDiscoveryTest.AdapterId", "discoveryAdapter");
            initData.properties.setProperty("AdapterForDiscoveryTest.Endpoints", "default");

            Communicator comm = Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            comm.stringToProxy("test @ TestAdapter").ice_ping();
            comm.stringToProxy("test").ice_ping();

            test(comm.getDefaultLocator().getRegistry() != null);
            test(
                LocatorPrx.uncheckedCast(comm.getDefaultLocator())
                    .getLocalRegistry()
                    != null);
            test(
                LocatorPrx.uncheckedCast(comm.getDefaultLocator())
                    .getLocalQuery()
                    != null);

            ObjectAdapter adapter =
                comm.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.activate();
            adapter.deactivate();
            comm.destroy();

            // Now, ensure that the IceGrid discovery locator correctly handles failure to find a
            // locator.
            initData.properties.setProperty("IceLocatorDiscovery.InstanceName", "unknown");
            initData.properties.setProperty("IceLocatorDiscovery.RetryCount", "1");
            initData.properties.setProperty("IceLocatorDiscovery.Timeout", "100");
            comm = Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            try {
                comm.stringToProxy("test @ TestAdapter").ice_ping();
            } catch (NoEndpointException ex) {}
            try {
                comm.stringToProxy("test").ice_ping();
            } catch (NoEndpointException ex) {}
            test(comm.getDefaultLocator().getRegistry() == null);
            test(LocatorPrx.checkedCast(comm.getDefaultLocator()) == null);
            try {
                LocatorPrx.uncheckedCast(comm.getDefaultLocator())
                    .getLocalQuery();
            } catch (OperationNotExistException ex) {}

            adapter = comm.createObjectAdapter("AdapterForDiscoveryTest");
            adapter.activate();
            adapter.deactivate();

            comm.destroy();

            String multicast;
            if ("1".equals(communicator.getProperties().getIceProperty("Ice.IPv6"))) {
                multicast = "\"ff15::1\"";
            } else {
                multicast = "239.255.0.1";
            }

            // Test invalid lookup endpoints
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty(
                "IceLocatorDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");
            comm = Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            try {
                comm.stringToProxy("test @ TestAdapter").ice_ping();
                test(false);
            } catch (NoEndpointException ex) {}
            comm.destroy();

            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("IceLocatorDiscovery.RetryCount", "0");
            initData.properties.setProperty(
                "IceLocatorDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");
            comm = Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            try {
                comm.stringToProxy("test @ TestAdapter").ice_ping();
                test(false);
            } catch (NoEndpointException ex) {}
            comm.destroy();

            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Default.Locator", "");
            initData.properties.setProperty("IceLocatorDiscovery.RetryCount", "1");
            {
                String intf = initData.properties.getIceProperty("IceLocatorDiscovery.Interface");
                if (!intf.isEmpty()) {
                    intf = " --interface \"" + intf + "\"";
                }
                String port = Integer.toString(helper.getTestPort(99));
                initData.properties.setProperty(
                    "IceLocatorDiscovery.Lookup",
                    "udp -h "
                        + multicast
                        + " --interface unknown:"
                        + "udp -h "
                        + multicast
                        + " -p "
                        + port
                        + intf);
            }
            comm = Util.initialize(initData);
            test(comm.getDefaultLocator() != null);
            try {
                comm.stringToProxy("test @ TestAdapter").ice_ping();
            } catch (NoEndpointException ex) {
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

    public static void allTestsWithDeploy(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
        test(base != null);
        ObjectPrx base2 = communicator.stringToProxy("test");
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
        ObjectPrx base10 = communicator.stringToProxy("test10 @ TestAdapter10");
        test(base10 != null);
        ObjectPrx base102 = communicator.stringToProxy("test10");
        test(base102 != null);
        try {
            base10.ice_ping();
            test(false);
        } catch (NoEndpointException ex) {}
        try {
            base102.ice_ping();
            test(false);
        } catch (NoEndpointException ex) {}
        base10 = base10.ice_encodingVersion(Util.Encoding_1_0);
        base102 = base102.ice_encodingVersion(Util.Encoding_1_0);
        base10.ice_ping();
        base102.ice_ping();
        out.println("ok");

        out.print("testing reference with unknown identity... ");
        out.flush();
        try {
            communicator.stringToProxy("unknown/unknown").ice_ping();
            test(false);
        } catch (NotRegisteredException ex) {
            test("object".equals(ex.kindOfObject));
            test("unknown/unknown".equals(ex.id));
        }
        out.println("ok");

        out.print("testing reference with unknown adapter... ");
        out.flush();
        try {
            communicator.stringToProxy("test @ TestAdapterUnknown").ice_ping();
            test(false);
        } catch (NotRegisteredException ex) {
            test("object adapter".equals(ex.kindOfObject));
            test("TestAdapterUnknown".equals(ex.id));
        }
        out.println("ok");

        RegistryPrx registry =
            RegistryPrx.checkedCast(
                communicator.stringToProxy(
                    communicator.getDefaultLocator().ice_getIdentity().category
                        + "/Registry"));
        test(registry != null);
        AdminSessionPrx session = null;
        try {
            session = registry.createAdminSession("foo", "bar");
        } catch (PermissionDeniedException e) {
            test(false);
        }

        AdminPrx admin = session.getAdmin();
        test(admin != null);

        try {
            admin.enableServer("server", false);
            admin.stopServer("server");
        } catch (ServerNotExistException ex) {
            test(false);
        } catch (ServerStopException ex) {
            test(false);
        } catch (NodeUnreachableException ex) {
            test(false);
        } catch (DeploymentException ex) {
            test(false);
        }

        out.print("testing whether server is still reachable... ");
        out.flush();
        try {
            obj = TestIntfPrx.checkedCast(base);
            test(false);
        } catch (NoEndpointException ex) {}
        try {
            obj2 = TestIntfPrx.checkedCast(base2);
            test(false);
        } catch (NoEndpointException ex) {}

        try {
            admin.enableServer("server", true);
        } catch (ServerNotExistException ex) {
            test(false);
        } catch (NodeUnreachableException ex) {
            test(false);
        } catch (DeploymentException ex) {
            test(false);
        }

        try {
            obj = TestIntfPrx.checkedCast(base);
        } catch (NoEndpointException ex) {
            test(false);
        }
        try {
            obj2 = TestIntfPrx.checkedCast(base2);
        } catch (NoEndpointException ex) {
            test(false);
        }
        out.println("ok");

        try {
            admin.stopServer("server");
        } catch (ServerNotExistException ex) {
            test(false);
        } catch (ServerStopException ex) {
            test(false);
        } catch (NodeUnreachableException ex) {
            test(false);
        } catch (DeploymentException ex) {
            test(false);
        }

        session.destroy();
    }

    private AllTests() {}
}
