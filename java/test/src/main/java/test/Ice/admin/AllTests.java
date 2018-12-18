// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.admin;

import java.io.PrintWriter;
import test.Ice.admin.Test.*;

import com.zeroc.Ice.LogMessageType;
import com.zeroc.Ice.ProcessPrx;
import com.zeroc.Ice.PropertiesAdminPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static void testFacets(com.zeroc.Ice.Communicator com, boolean builtInFacets)
    {
        if(builtInFacets)
        {
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

        java.util.Map<String, com.zeroc.Ice.Object> facetMap = com.findAllAdminFacets();
        if(builtInFacets)
        {
            test(facetMap.size() == 7);
            test(facetMap.containsKey("Properties"));
            test(facetMap.containsKey("Process"));
            test(facetMap.containsKey("Logger"));
            test(facetMap.containsKey("Metrics"));
        }
        else
        {
            test(facetMap.size() >= 3);
        }
        test(facetMap.containsKey("Facet1"));
        test(facetMap.containsKey("Facet2"));
        test(facetMap.containsKey("Facet3"));

        try
        {
            com.addAdminFacet(f1, "Facet1");
            test(false);
        }
        catch(com.zeroc.Ice.AlreadyRegisteredException ex)
        {
            // Expected
        }

        try
        {
            com.removeAdminFacet("Bogus");
            test(false);
        }
        catch(com.zeroc.Ice.NotRegisteredException ex)
        {
            // Expected
        }

        com.removeAdminFacet("Facet1");
        com.removeAdminFacet("Facet2");
        com.removeAdminFacet("Facet3");

        try
        {
            com.removeAdminFacet("Facet1");
            test(false);
        }
        catch(com.zeroc.Ice.NotRegisteredException ex)
        {
            // Expected
        }
    }

    public static void allTests(test.TestHelper helper)
    {
        PrintWriter out = helper.getWriter();

        out.print("testing communicator operations... ");
        out.flush();
        {
            //
            // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
            //
            com.zeroc.Ice.InitializationData init = new com.zeroc.Ice.InitializationData();
            init.properties = com.zeroc.Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            try(com.zeroc.Ice.Communicator comm = com.zeroc.Ice.Util.initialize(init))
            {
                testFacets(comm, true);
            }
        }
        {
            //
            // Test: Verify that the operations work correctly in the presence of facet filters.
            //
            com.zeroc.Ice.InitializationData init = new com.zeroc.Ice.InitializationData();
            init.properties = com.zeroc.Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.Facets", "Properties");
            try(com.zeroc.Ice.Communicator comm = com.zeroc.Ice.Util.initialize(init))
            {
                testFacets(comm, false);
            }
        }
        {
            //
            // Test: Verify that the operations work correctly with the Admin object disabled.
            //
            try(com.zeroc.Ice.Communicator comm = com.zeroc.Ice.Util.initialize())
            {
                testFacets(comm, false);
            }
        }
        {
            //
            // Test: Verify that the operations work correctly when Ice.Admin.Enabled is set
            //
            com.zeroc.Ice.InitializationData init = new com.zeroc.Ice.InitializationData();
            init.properties = com.zeroc.Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Enabled", "1");
            try(com.zeroc.Ice.Communicator comm = com.zeroc.Ice.Util.initialize(init))
            {
                test(comm.getAdmin() == null);
                com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("test-admin");
                try
                {
                    comm.createAdmin(null, id);
                    test(false);
                }
                catch(com.zeroc.Ice.InitializationException ex)
                {
                }

                com.zeroc.Ice.ObjectAdapter adapter = comm.createObjectAdapter("");
                test(comm.createAdmin(adapter, id) != null);
                test(comm.getAdmin() != null);
                testFacets(comm, true);
            }
        }
        {
            //
            // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
            //
            com.zeroc.Ice.InitializationData init = new com.zeroc.Ice.InitializationData();
            init.properties = com.zeroc.Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.DelayCreation", "1");
            try(com.zeroc.Ice.Communicator comm = com.zeroc.Ice.Util.initialize(init))
            {
                testFacets(comm, true);
                comm.getAdmin();
                testFacets(comm, true);
                comm.destroy();
            }
        }
        out.println("ok");

        String ref = "factory:" + helper.getTestEndpoint(0) + " -t 10000";
        RemoteCommunicatorFactoryPrx factory =
            RemoteCommunicatorFactoryPrx.uncheckedCast(helper.communicator().stringToProxy(ref));

        out.print("testing process facet... ");
        out.flush();
        {
            //
            // Test: Verify that Process::shutdown() operation shuts down the communicator.
            //
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
            ProcessPrx proc = ProcessPrx.checkedCast(obj, "Process");
            proc.shutdown();
            rcom.waitForShutdown();
            rcom.destroy();
        }
        out.println("ok");

        out.print("testing properties facet... ");
        out.flush();
        {
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Prop1", "1");
            props.put("Prop2", "2");
            props.put("Prop3", "3");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
            PropertiesAdminPrx pa = PropertiesAdminPrx.checkedCast(obj, "Properties");

            //
            // Test: PropertiesAdmin::getProperty()
            //
            test(pa.getProperty("Prop2").equals("2"));
            test(pa.getProperty("Bogus").equals(""));

            //
            // Test: PropertiesAdmin::getProperties()
            //
            java.util.Map<String, String> pd = pa.getPropertiesForPrefix("");
            test(pd.size() == 5);
            test(pd.get("Ice.Admin.Endpoints").equals("tcp -h 127.0.0.1"));
            test(pd.get("Ice.Admin.InstanceName").equals("Test"));
            test(pd.get("Prop1").equals("1"));
            test(pd.get("Prop2").equals("2"));
            test(pd.get("Prop3").equals("3"));

            java.util.Map<String, String> changes;

            //
            // Test: PropertiesAdmin::setProperties()
            //
            java.util.Map<String, String> setProps = new java.util.HashMap<>();
            setProps.put("Prop1", "10"); // Changed
            setProps.put("Prop2", "20"); // Changed
            setProps.put("Prop3", ""); // Removed
            setProps.put("Prop4", "4"); // Added
            setProps.put("Prop5", "5"); // Added
            pa.setProperties(setProps);
            test(pa.getProperty("Prop1").equals("10"));
            test(pa.getProperty("Prop2").equals("20"));
            test(pa.getProperty("Prop3").equals(""));
            test(pa.getProperty("Prop4").equals("4"));
            test(pa.getProperty("Prop5").equals("5"));
            changes = rcom.getChanges();
            test(changes.size() == 5);
            test(changes.get("Prop1").equals("10"));
            test(changes.get("Prop2").equals("20"));
            test(changes.get("Prop3").equals(""));
            test(changes.get("Prop4").equals("4"));
            test(changes.get("Prop5").equals("5"));
            pa.setProperties(setProps);
            changes = rcom.getChanges();
            test(changes.isEmpty());

            rcom.destroy();
        }
        out.println("ok");

        out.print("testing logger facet... ");
        out.flush();
        {
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("NullLogger", "1");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);

            rcom.trace("testCat", "trace");
            rcom.warning("warning");
            rcom.error("error");
            rcom.print("print");

            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
            com.zeroc.Ice.LoggerAdminPrx logger = com.zeroc.Ice.LoggerAdminPrx.checkedCast(obj, "Logger");
            test(logger != null);

            //
            // Get all
            //
            com.zeroc.Ice.LoggerAdmin.GetLogResult r = logger.getLog(null, null, -1);

            test(r.returnValue.length == 4);
            test(r.prefix.equals("NullLogger"));
            test(r.returnValue[0].traceCategory.equals("testCat") && r.returnValue[0].message.equals("trace"));
            test(r.returnValue[1].message.equals("warning"));
            test(r.returnValue[2].message.equals("error"));
            test(r.returnValue[3].message.equals("print"));

            //
            // Get only errors and warnings
            //
            rcom.error("error2");
            rcom.print("print2");
            rcom.trace("testCat", "trace2");
            rcom.warning("warning2");

            LogMessageType[] messageTypes =
                {
                    LogMessageType.ErrorMessage,
                    LogMessageType.WarningMessage
                };

            r = logger.getLog(messageTypes, null, -1);
            test(r.returnValue.length == 4);
            test(r.prefix.equals("NullLogger"));

            for(com.zeroc.Ice.LogMessage msg : java.util.Arrays.asList(r.returnValue))
            {
                test(msg.type == LogMessageType.ErrorMessage ||
                     msg.type == LogMessageType.WarningMessage);
            }

            //
            // Get only errors and traces with Cat = "testCat"
            //
            rcom.trace("testCat2", "A");
            rcom.trace("testCat", "trace3");
            rcom.trace("testCat2", "B");

            messageTypes = new LogMessageType[]
                {
                    LogMessageType.ErrorMessage,
                    LogMessageType.TraceMessage
                };
            String[] categories = {"testCat"};
            r = logger.getLog(messageTypes, categories, -1);
            test(r.returnValue.length == 5);
            test(r.prefix.equals("NullLogger"));

            for(com.zeroc.Ice.LogMessage msg : java.util.Arrays.asList(r.returnValue))
            {
                test(msg.type == LogMessageType.ErrorMessage ||
                     (msg.type == LogMessageType.TraceMessage && msg.traceCategory.equals("testCat")));
            }

            //
            // Same, but limited to last 2 messages (trace3 + error3)
            //
            rcom.error("error3");

            r = logger.getLog(messageTypes, categories, 2);
            test(r.returnValue.length == 2);
            test(r.prefix.equals("NullLogger"));

            test(r.returnValue[0].message.equals("trace3"));
            test(r.returnValue[1].message.equals("error3"));

            //
            // Now, test RemoteLogger
            //
            com.zeroc.Ice.ObjectAdapter adapter =
                helper.communicator().createObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");

            RemoteLoggerI remoteLogger = new RemoteLoggerI();

            com.zeroc.Ice.RemoteLoggerPrx myProxy =
                com.zeroc.Ice.RemoteLoggerPrx.uncheckedCast(adapter.addWithUUID(remoteLogger));

            adapter.activate();

            //
            // No filtering
            //
            r = logger.getLog(null, null, -1);
            remoteLogger.checkNextInit(r.prefix, r.returnValue);

            try
            {
                logger.attachRemoteLogger(myProxy, null, null, -1);
            }
            catch(com.zeroc.Ice.RemoteLoggerAlreadyAttachedException ex)
            {
                test(false);
            }

            remoteLogger.wait(1);

            remoteLogger.checkNextLog(LogMessageType.TraceMessage, "rtrace", "testCat");
            remoteLogger.checkNextLog(LogMessageType.WarningMessage, "rwarning", "");
            remoteLogger.checkNextLog(LogMessageType.ErrorMessage, "rerror", "");
            remoteLogger.checkNextLog(LogMessageType.PrintMessage, "rprint", "");

            rcom.trace("testCat", "rtrace");
            rcom.warning("rwarning");
            rcom.error("rerror");
            rcom.print("rprint");

            remoteLogger.wait(4);

            test(logger.detachRemoteLogger(myProxy));
            test(!logger.detachRemoteLogger(myProxy));

            //
            // Use Error + Trace with "traceCat" filter with 4 limit
            //
            r = logger.getLog(messageTypes, categories, 4);
            test(r.returnValue.length == 4);
            remoteLogger.checkNextInit(r.prefix, r.returnValue);

            try
            {
                logger.attachRemoteLogger(myProxy, messageTypes, categories, 4);
            }
            catch(com.zeroc.Ice.RemoteLoggerAlreadyAttachedException ex)
            {
                test(false);
            }

            remoteLogger.wait(1);

            remoteLogger.checkNextLog(LogMessageType.TraceMessage, "rtrace2", "testCat");
            remoteLogger.checkNextLog(LogMessageType.ErrorMessage, "rerror2", "");

            rcom.warning("rwarning2");
            rcom.trace("testCat", "rtrace2");
            rcom.warning("rwarning3");
            rcom.error("rerror2");
            rcom.print("rprint2");

            remoteLogger.wait(2);

            //
            // Attempt reconnection with slightly different proxy
            //
            try
            {
                logger.attachRemoteLogger(myProxy.ice_oneway(), messageTypes, categories, 4);
                test(false);
            }
            catch(com.zeroc.Ice.RemoteLoggerAlreadyAttachedException ex)
            {
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
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
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
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Properties");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
            ProcessPrx proc = ProcessPrx.checkedCast(obj, "Process");
            test(proc == null);
            TestFacetPrx tf = TestFacetPrx.checkedCast(obj, "TestFacet");
            test(tf == null);
            rcom.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the Process facet,
            // meaning no other facet is available.
            //
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Process");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
            PropertiesAdminPrx pa = PropertiesAdminPrx.checkedCast(obj, "Properties");
            test(pa == null);
            TestFacetPrx tf = TestFacetPrx.checkedCast(obj, "TestFacet");
            test(tf == null);
            rcom.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
            // meaning no other facet is available.
            //
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "TestFacet");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
            PropertiesAdminPrx pa = PropertiesAdminPrx.checkedCast(obj, "Properties");
            test(pa == null);
            ProcessPrx proc = ProcessPrx.checkedCast(obj, "Process");
            test(proc == null);
            rcom.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
            // facet names.
            //
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Properties TestFacet");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
            PropertiesAdminPrx pa = PropertiesAdminPrx.checkedCast(obj, "Properties");
            test(pa.getProperty("Ice.Admin.InstanceName").equals("Test"));
            TestFacetPrx tf = TestFacetPrx.checkedCast(obj, "TestFacet");
            tf.op();
            ProcessPrx proc = ProcessPrx.checkedCast(obj, "Process");
            test(proc == null);
            rcom.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
            // facet names.
            //
            java.util.Map<String, String> props = new java.util.HashMap<>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "TestFacet, Process");
            RemoteCommunicatorPrx rcom = factory.createCommunicator(props);
            com.zeroc.Ice.ObjectPrx obj = rcom.getAdmin();
            PropertiesAdminPrx pa = PropertiesAdminPrx.checkedCast(obj, "Properties");
            test(pa == null);
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
}
