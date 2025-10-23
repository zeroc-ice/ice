// Copyright (c) ZeroC, Inc.

package test.Ice.admin;

import com.zeroc.Ice.AlreadyRegisteredException;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.FacetNotExistException;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.InitializationException;
import com.zeroc.Ice.LogMessage;
import com.zeroc.Ice.LogMessageType;
import com.zeroc.Ice.LoggerAdmin;
import com.zeroc.Ice.LoggerAdminPrx;
import com.zeroc.Ice.NotRegisteredException;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.ProcessPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.PropertiesAdminPrx;
import com.zeroc.Ice.RemoteLoggerAlreadyAttachedException;
import com.zeroc.Ice.RemoteLoggerPrx;

import test.Ice.admin.Test.RemoteCommunicatorFactoryPrx;
import test.Ice.admin.Test.RemoteCommunicatorPrx;
import test.Ice.admin.Test.TestFacet;
import test.Ice.admin.Test.TestFacetPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    static void testFacets(Communicator com, boolean builtInFacets) {
        if (builtInFacets) {
            test(com.findAdminFacet("Properties") != null);
            test(com.findAdminFacet("Process") != null);
            test(com.findAdminFacet("Logger") != null);
            test(com.findAdminFacet("Metrics") != null);
        }

        TestFacet f1 = new TestFacetI();
        TestFacet f2 = new TestFacetI();
        TestFacet f3 = new TestFacetI();

        com.addAdminFacet(f1, "Facet1");
        com.addAdminFacet(f2, "Facet2");
        com.addAdminFacet(f3, "Facet3");

        test(com.findAdminFacet("Facet1") == f1);
        test(com.findAdminFacet("Facet2") == f2);
        test(com.findAdminFacet("Facet3") == f3);
        test(com.findAdminFacet("Bogus") == null);

        Map<String, Object> facetMap = com.findAllAdminFacets();
        if (builtInFacets) {
            test(facetMap.size() == 7);
            test(facetMap.containsKey("Properties"));
            test(facetMap.containsKey("Process"));
            test(facetMap.containsKey("Logger"));
            test(facetMap.containsKey("Metrics"));
        } else {
            test(facetMap.size() >= 3);
        }
        test(facetMap.containsKey("Facet1"));
        test(facetMap.containsKey("Facet2"));
        test(facetMap.containsKey("Facet3"));

        try {
            com.addAdminFacet(f1, "Facet1");
            test(false);
        } catch (AlreadyRegisteredException ex) {
            // Expected
        }

        try {
            com.removeAdminFacet("Bogus");
            test(false);
        } catch (NotRegisteredException ex) {
            // Expected
        }

        com.removeAdminFacet("Facet1");
        com.removeAdminFacet("Facet2");
        com.removeAdminFacet("Facet3");

        try {
            com.removeAdminFacet("Facet1");
            test(false);
        } catch (NotRegisteredException ex) {
            // Expected
        }
    }

    public static void allTests(TestHelper helper) {
        PrintWriter out = helper.getWriter();

        out.print("testing communicator operations... ");
        out.flush();
        {
            //
            // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical
            // configuration.
            //
            InitializationData init = new InitializationData();
            init.properties = new Properties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            try (Communicator comm = new Communicator(init)) {
                testFacets(comm, true);
            }
        }
        {
            //
            // Test: Verify that the operations work correctly in the presence of facet filters.
            //
            InitializationData init = new InitializationData();
            init.properties = new Properties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.Facets", "Properties");
            try (Communicator comm = new Communicator(init)) {
                testFacets(comm, false);
            }
        }
        {
            //
            // Test: Verify that the operations work correctly with the Admin object disabled.
            //
            try (Communicator comm = new Communicator()) {
                testFacets(comm, false);
            }
        }
        {
            //
            // Test: Verify that the operations work correctly when Ice.Admin.Enabled is set
            //
            InitializationData init = new InitializationData();
            init.properties = new Properties();
            init.properties.setProperty("Ice.Admin.Enabled", "1");
            try (Communicator comm = new Communicator(init)) {
                test(comm.getAdmin() == null);
                Identity id = new Identity("test-admin", "");
                try {
                    comm.createAdmin(null, id);
                    test(false);
                } catch (InitializationException ex) {}

                ObjectAdapter adapter = comm.createObjectAdapter("");
                test(comm.createAdmin(adapter, id) != null);
                test(comm.getAdmin() != null);
                testFacets(comm, true);
            }
        }
        {
            //
            // Test: Verify that the operations work correctly when creation of the Admin object is
            // delayed.
            //
            InitializationData init = new InitializationData();
            init.properties = new Properties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.DelayCreation", "1");
            try (Communicator comm = new Communicator(init)) {
                testFacets(comm, true);
                comm.getAdmin();
                testFacets(comm, true);
                comm.destroy();
            }
        }
        out.println("ok");

        String ref = "factory:" + helper.getTestEndpoint(0) + " -t 10000";
        var factory = RemoteCommunicatorFactoryPrx.createProxy(helper.communicator(), ref);

        out.print("testing process facet... ");
        out.flush();
        {
            //
            // Test: Verify that Process::shutdown() operation shuts down the communicator.
            //
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            ObjectPrx obj = rcom.getAdmin();
            ProcessPrx proc = ProcessPrx.checkedCast(obj, "Process");
            proc.shutdown();
            rcom.waitForShutdown();
            rcom.destroy();
        }
        out.println("ok");

        out.print("testing properties facet... ");
        out.flush();
        {
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Prop1", "1");
            props.put("Prop2", "2");
            props.put("Prop3", "3");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            ObjectPrx obj = rcom.getAdmin();
            PropertiesAdminPrx pa = PropertiesAdminPrx.checkedCast(obj, "Properties");

            //
            // Test: PropertiesAdmin::getProperty()
            //
            test("2".equals(pa.getProperty("Prop2")));
            test(pa.getProperty("Bogus").isEmpty());

            //
            // Test: PropertiesAdmin::getProperties()
            //
            Map<String, String> pd = pa.getPropertiesForPrefix("");
            test(pd.size() == 5);
            test("tcp -h 127.0.0.1".equals(pd.get("Ice.Admin.Endpoints")));
            test("Test".equals(pd.get("Ice.Admin.InstanceName")));
            test("1".equals(pd.get("Prop1")));
            test("2".equals(pd.get("Prop2")));
            test("3".equals(pd.get("Prop3")));

            Map<String, String> changes;

            //
            // Test: PropertiesAdmin::setProperties()
            //
            Map<String, String> setProps = new HashMap<>();
            setProps.put("Prop1", "10"); // Changed
            setProps.put("Prop2", "20"); // Changed
            setProps.put("Prop3", ""); // Removed
            setProps.put("Prop4", "4"); // Added
            setProps.put("Prop5", "5"); // Added
            pa.setProperties(setProps);
            test("10".equals(pa.getProperty("Prop1")));
            test("20".equals(pa.getProperty("Prop2")));
            test(pa.getProperty("Prop3").isEmpty());
            test("4".equals(pa.getProperty("Prop4")));
            test("5".equals(pa.getProperty("Prop5")));
            changes = rcom.getChanges();
            test(changes.size() == 5);
            test("10".equals(changes.get("Prop1")));
            test("20".equals(changes.get("Prop2")));
            test(changes.get("Prop3").isEmpty());
            test("4".equals(changes.get("Prop4")));
            test("5".equals(changes.get("Prop5")));
            pa.setProperties(setProps);
            changes = rcom.getChanges();
            test(changes.isEmpty());

            rcom.destroy();
        }
        out.println("ok");

        out.print("testing logger facet... ");
        out.flush();
        {
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("NullLogger", "1");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);

            rcom.trace("testCat", "trace");
            rcom.warning("warning");
            rcom.error("error");
            rcom.print("print");

            ObjectPrx obj = rcom.getAdmin();
            LoggerAdminPrx logger =
                LoggerAdminPrx.checkedCast(obj, "Logger");
            test(logger != null);

            //
            // Get all
            //
            LoggerAdmin.GetLogResult r = logger.getLog(null, null, -1);

            test(r.returnValue.length == 4);
            test("NullLogger".equals(r.prefix));
            test(
                "testCat".equals(r.returnValue[0].traceCategory)
                    && "trace".equals(r.returnValue[0].message));
            test("warning".equals(r.returnValue[1].message));
            test("error".equals(r.returnValue[2].message));
            test("print".equals(r.returnValue[3].message));

            //
            // Get only errors and warnings
            //
            rcom.error("error2");
            rcom.print("print2");
            rcom.trace("testCat", "trace2");
            rcom.warning("warning2");

            LogMessageType[] messageTypes = {
                LogMessageType.ErrorMessage, LogMessageType.WarningMessage
            };

            r = logger.getLog(messageTypes, null, -1);
            test(r.returnValue.length == 4);
            test("NullLogger".equals(r.prefix));

            for (LogMessage msg : Arrays.asList(r.returnValue)) {
                test(
                    msg.type == LogMessageType.ErrorMessage
                        || msg.type == LogMessageType.WarningMessage);
            }

            //
            // Get only errors and traces with Cat = "testCat"
            //
            rcom.trace("testCat2", "A");
            rcom.trace("testCat", "trace3");
            rcom.trace("testCat2", "B");

            messageTypes =
                new LogMessageType[]{LogMessageType.ErrorMessage, LogMessageType.TraceMessage};
            String[] categories = {"testCat"};
            r = logger.getLog(messageTypes, categories, -1);
            test(r.returnValue.length == 5);
            test("NullLogger".equals(r.prefix));

            for (LogMessage msg : Arrays.asList(r.returnValue)) {
                test(
                    msg.type == LogMessageType.ErrorMessage
                        || (msg.type == LogMessageType.TraceMessage
                        && "testCat".equals(msg.traceCategory)));
            }

            //
            // Same, but limited to last 2 messages (trace3 + error3)
            //
            rcom.error("error3");

            r = logger.getLog(messageTypes, categories, 2);
            test(r.returnValue.length == 2);
            test("NullLogger".equals(r.prefix));

            test("trace3".equals(r.returnValue[0].message));
            test("error3".equals(r.returnValue[1].message));

            //
            // Now, test RemoteLogger
            //
            ObjectAdapter adapter =
                helper.communicator()
                    .createObjectAdapterWithEndpoints(
                        "RemoteLoggerAdapter", "tcp -h localhost");

            RemoteLoggerI remoteLogger = new RemoteLoggerI();

            RemoteLoggerPrx myProxy =
                RemoteLoggerPrx.uncheckedCast(adapter.addWithUUID(remoteLogger));

            adapter.activate();

            //
            // No filtering
            //
            r = logger.getLog(null, null, -1);
            try {
                logger.attachRemoteLogger(myProxy, null, null, -1);
            } catch (RemoteLoggerAlreadyAttachedException ex) {
                test(false);
            }
            remoteLogger.wait(1);

            for (int i = 0; i < r.returnValue.length; i++) {
                LogMessage m = r.returnValue[i];
                remoteLogger.checkNextInit(r.prefix, m.type, m.message, m.traceCategory);
            }

            rcom.trace("testCat", "rtrace");
            rcom.warning("rwarning");
            rcom.error("rerror");
            rcom.print("rprint");

            remoteLogger.wait(4);

            remoteLogger.checkNextLog(LogMessageType.TraceMessage, "rtrace", "testCat");
            remoteLogger.checkNextLog(LogMessageType.WarningMessage, "rwarning", "");
            remoteLogger.checkNextLog(LogMessageType.ErrorMessage, "rerror", "");
            remoteLogger.checkNextLog(LogMessageType.PrintMessage, "rprint", "");

            test(logger.detachRemoteLogger(myProxy));
            test(!logger.detachRemoteLogger(myProxy));

            //
            // Use Error + Trace with "traceCat" filter with 4 limit
            //
            r = logger.getLog(messageTypes, categories, 4);
            test(r.returnValue.length == 4);

            try {
                logger.attachRemoteLogger(myProxy, messageTypes, categories, 4);
            } catch (RemoteLoggerAlreadyAttachedException ex) {
                test(false);
            }
            remoteLogger.wait(1);

            for (int i = 0; i < r.returnValue.length; i++) {
                LogMessage m = r.returnValue[i];
                remoteLogger.checkNextInit(r.prefix, m.type, m.message, m.traceCategory);
            }

            rcom.warning("rwarning2");
            rcom.trace("testCat", "rtrace2");
            rcom.warning("rwarning3");
            rcom.error("rerror2");
            rcom.print("rprint2");

            remoteLogger.wait(2);

            remoteLogger.checkNextLog(LogMessageType.TraceMessage, "rtrace2", "testCat");
            remoteLogger.checkNextLog(LogMessageType.ErrorMessage, "rerror2", "");
            //
            // Attempt reconnection with slightly different proxy
            //
            try {
                logger.attachRemoteLogger(myProxy.ice_oneway(), messageTypes, categories, 4);
                test(false);
            } catch (RemoteLoggerAlreadyAttachedException ex) {
                // expected
            }

            rcom.destroy();
        }
        out.println("ok");

        out.print("testing custom facet... ");
        out.flush();
        {
            //
            // Test: Verify that the custom facet is present.
            //
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            ObjectPrx obj = rcom.getAdmin();
            TestFacetPrx tf = TestFacetPrx.checkedCast(obj, "TestFacet");
            tf.op();
            rcom.destroy();
        }
        out.println("ok");

        out.print("testing facet filtering... ");
        out.flush();
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the Properties facet,
            // meaning no other facet is available.
            //
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Properties");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            ObjectPrx obj = rcom.getAdmin();

            try {
                ProcessPrx.checkedCast(obj, "Process");
                test(false);
            } catch (FacetNotExistException ex) {
                // expected
            }
            try {
                TestFacetPrx.checkedCast(obj, "TestFacet");
                test(false);
            } catch (FacetNotExistException ex) {
                // expected
            }
            rcom.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the Process facet,
            // meaning no other facet is available.
            //
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Process");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            ObjectPrx obj = rcom.getAdmin();

            try {
                PropertiesAdminPrx.checkedCast(obj, "Properties");
                test(false);
            } catch (FacetNotExistException ex) {
                // expected
            }
            try {
                TestFacetPrx.checkedCast(obj, "TestFacet");
                test(false);
            } catch (FacetNotExistException ex) {
                // expected
            }

            rcom.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
            // meaning no other facet is available.
            //
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "TestFacet");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            ObjectPrx obj = rcom.getAdmin();
            try {
                PropertiesAdminPrx.checkedCast(obj, "Properties");
                test(false);
            } catch (FacetNotExistException ex) {
                // expected
            }
            try {
                ProcessPrx.checkedCast(obj, "Process");
                test(false);
            } catch (FacetNotExistException ex) {
                // expected
            }
            rcom.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
            // facet names.
            //
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Properties TestFacet");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            ObjectPrx obj = rcom.getAdmin();
            PropertiesAdminPrx pa = PropertiesAdminPrx.checkedCast(obj, "Properties");
            test("Test".equals(pa.getProperty("Ice.Admin.InstanceName")));
            TestFacetPrx tf = TestFacetPrx.checkedCast(obj, "TestFacet");
            tf.op();
            try {
                ProcessPrx.checkedCast(obj, "Process");
                test(false);
            } catch (FacetNotExistException ex) {
                // expected
            }
            rcom.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
            // facet names.
            //
            Map<String, String> props = new HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "TestFacet, Process");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            ObjectPrx obj = rcom.getAdmin();
            try {
                PropertiesAdminPrx.checkedCast(obj, "Properties");
                test(false);
            } catch (FacetNotExistException ex) {
                // expected
            }
            TestFacetPrx tf = TestFacetPrx.checkedCast(obj, "TestFacet");
            tf.op();
            ProcessPrx proc = ProcessPrx.checkedCast(obj, "Process");
            proc.shutdown();
            rcom.waitForShutdown();
            rcom.destroy();
        }
        out.println("ok");
        out.flush();

        factory.shutdown();
    }

    private AllTests() {}
}
