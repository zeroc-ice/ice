// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.admin;

import java.io.PrintWriter;
import test.Ice.admin.Test.*;

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

    static void
    testFacets(Ice.Communicator com, boolean builtInFacets)
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

        java.util.Map<String, Ice.Object> facetMap = com.findAllAdminFacets();
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
        catch(Ice.AlreadyRegisteredException ex)
        {
            // Expected
        }

        try
        {
            com.removeAdminFacet("Bogus");
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
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
        catch(Ice.NotRegisteredException ex)
        {
            // Expected
        }
    }

    public static void
    allTests(test.Util.Application app, PrintWriter out)
    {
        out.print("testing communicator operations... ");
        out.flush();
        {
            //
            // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
            //
            Ice.InitializationData init = app.createInitializationData();
            init.properties = Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            Ice.Communicator com = Ice.Util.initialize(init);
            testFacets(com, true);
            com.destroy();
        }
        {
            //
            // Test: Verify that the operations work correctly in the presence of facet filters.
            //
            Ice.InitializationData init = app.createInitializationData();
            init.properties = Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.Facets", "Properties");
            Ice.Communicator com = Ice.Util.initialize(init);
            testFacets(com, false);
            com.destroy();
        }
        {
            //
            // Test: Verify that the operations work correctly with the Admin object disabled.
            //
            Ice.Communicator com = Ice.Util.initialize();
            testFacets(com, false);
            com.destroy();
        }
        {
            //
            // Test: Verify that the operations work correctly when Ice.Admin.Enabled is set
            //
            Ice.InitializationData init = app.createInitializationData();
            init.properties = Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Enabled", "1");
            Ice.Communicator com = Ice.Util.initialize(init);

            test(com.getAdmin() == null);
            Ice.Identity id = Ice.Util.stringToIdentity("test-admin");
            try
            {
                com.createAdmin(null, id);
                test(false);
            }
            catch(Ice.InitializationException ex)
            {
            }

            Ice.ObjectAdapter adapter = com.createObjectAdapter("");
            test(com.createAdmin(adapter, id) != null);
            test(com.getAdmin() != null);
            testFacets(com, true);
            com.destroy();
        }
        {
            //
            // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
            //
            Ice.InitializationData init = app.createInitializationData();
            init.properties = Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.DelayCreation", "1");
            Ice.Communicator com = Ice.Util.initialize(init);
            testFacets(com, true);
            com.getAdmin();
            testFacets(com, true);
            com.destroy();
        }
        out.println("ok");

        String ref = "factory:default -p 12010 -t 10000";
        RemoteCommunicatorFactoryPrx factory =
            RemoteCommunicatorFactoryPrxHelper.uncheckedCast(app.communicator().stringToProxy(ref));

        out.print("testing process facet... ");
        out.flush();
        {
            //
            // Test: Verify that Process::shutdown() operation shuts down the communicator.
            //
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.ProcessPrx proc = Ice.ProcessPrxHelper.checkedCast(obj, "Process");
            proc.shutdown();
            com.waitForShutdown();
            com.destroy();
        }
        out.println("ok");

        out.print("testing properties facet... ");
        out.flush();
        {
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Prop1", "1");
            props.put("Prop2", "2");
            props.put("Prop3", "3");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");

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
            java.util.Map<String, String> setProps = new java.util.HashMap<String, String>();
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
            changes = com.getChanges();
            test(changes.size() == 5);
            test(changes.get("Prop1").equals("10"));
            test(changes.get("Prop2").equals("20"));
            test(changes.get("Prop3").equals(""));
            test(changes.get("Prop4").equals("4"));
            test(changes.get("Prop5").equals("5"));
            pa.setProperties(setProps);
            changes = com.getChanges();
            test(changes.isEmpty());

            com.destroy();
        }
        out.println("ok");

        out.print("testing logger facet... ");
        out.flush();
        {
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("NullLogger", "1");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            
            com.trace("testCat", "trace");
            com.warning("warning");
            com.error("error");
            com.print("print");
       
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.LoggerAdminPrx logger = Ice.LoggerAdminPrxHelper.checkedCast(obj, "Logger");
            test(logger != null);

            Ice.StringHolder prefix = new Ice.StringHolder(); 
            
            //
            // Get all
            //
            Ice.LogMessage[] logMessages = logger.getLog(null, null, -1, prefix);
            
            test(logMessages.length == 4);
            test(prefix.value.equals("NullLogger"));
            test(logMessages[0].traceCategory.equals("testCat") && logMessages[0].message.equals("trace")); 
            test(logMessages[1].message.equals("warning"));
            test(logMessages[2].message.equals("error"));
            test(logMessages[3].message.equals("print"));
       
            //
            // Get only errors and warnings
            //
            com.error("error2");
            com.print("print2");
            com.trace("testCat", "trace2");
            com.warning("warning2");
            
            Ice.LogMessageType[] messageTypes = {Ice.LogMessageType.ErrorMessage, Ice.LogMessageType.WarningMessage};
          
            logMessages = logger.getLog(messageTypes, null, -1, prefix);
            test(logMessages.length == 4);
            test(prefix.value.equals("NullLogger"));
            
            for(Ice.LogMessage msg : java.util.Arrays.asList(logMessages))
            {
                test(msg.type == Ice.LogMessageType.ErrorMessage || msg.type == Ice.LogMessageType.WarningMessage);
            }
       
            //
            // Get only errors and traces with Cat = "testCat"
            //
            com.trace("testCat2", "A");
            com.trace("testCat", "trace3");
            com.trace("testCat2", "B");
            
            messageTypes = new Ice.LogMessageType[]{Ice.LogMessageType.ErrorMessage, Ice.LogMessageType.TraceMessage};
            String[] categories = {"testCat"};
            logMessages = logger.getLog(messageTypes, categories, -1, prefix);
            test(logMessages.length == 5);
            test(prefix.value.equals("NullLogger"));
            
            for(Ice.LogMessage msg : java.util.Arrays.asList(logMessages))
            { 
                test(msg.type == Ice.LogMessageType.ErrorMessage || 
                     (msg.type == Ice.LogMessageType.TraceMessage && msg.traceCategory.equals("testCat")));
            }

            //
            // Same, but limited to last 2 messages (trace3 + error3)
            //
            com.error("error3");

            logMessages = logger.getLog(messageTypes, categories, 2, prefix);
            test(logMessages.length == 2);
            test(prefix.value.equals("NullLogger"));
            
            test(logMessages[0].message.equals("trace3"));
            test(logMessages[1].message.equals("error3"));
              
            //
            // Now, test RemoteLogger
            //
            Ice.ObjectAdapter adapter = 
                app.communicator().createObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");
   
            RemoteLoggerI remoteLogger = new RemoteLoggerI();
        
            Ice.RemoteLoggerPrx myProxy = Ice.RemoteLoggerPrxHelper.uncheckedCast(adapter.addWithUUID(remoteLogger));
        
            adapter.activate();

            //
            // No filtering
            //
            logMessages = logger.getLog(null, null, -1, prefix);
            remoteLogger.checkNextInit(prefix.value, logMessages);
            
            try
            {
                logger.attachRemoteLogger(myProxy, null, null, -1);
            }
            catch(Ice.RemoteLoggerAlreadyAttachedException ex)
            {
                test(false);
            }

            remoteLogger.wait(1);
            
            remoteLogger.checkNextLog(Ice.LogMessageType.TraceMessage, "rtrace", "testCat");
            remoteLogger.checkNextLog(Ice.LogMessageType.WarningMessage, "rwarning", "");
            remoteLogger.checkNextLog(Ice.LogMessageType.ErrorMessage, "rerror", "");
            remoteLogger.checkNextLog(Ice.LogMessageType.PrintMessage, "rprint", "");
            
            com.trace("testCat", "rtrace");
            com.warning("rwarning");
            com.error("rerror");
            com.print("rprint");

            remoteLogger.wait(4);
            
            test(logger.detachRemoteLogger(myProxy));
            test(!logger.detachRemoteLogger(myProxy));
           
            //
            // Use Error + Trace with "traceCat" filter with 4 limit
            //
            logMessages = logger.getLog(messageTypes, categories, 4, prefix);
            test(logMessages.length == 4);
            remoteLogger.checkNextInit(prefix.value, logMessages);

            try
            {
                logger.attachRemoteLogger(myProxy, messageTypes, categories, 4);
            }
            catch(Ice.RemoteLoggerAlreadyAttachedException ex)
            {
                test(false);
            }

            remoteLogger.wait(1);
            
            remoteLogger.checkNextLog(Ice.LogMessageType.TraceMessage, "rtrace2", "testCat");
            remoteLogger.checkNextLog(Ice.LogMessageType.ErrorMessage, "rerror2", "");
            
            com.warning("rwarning2");
            com.trace("testCat", "rtrace2");
            com.warning("rwarning3");
            com.error("rerror2");
            com.print("rprint2");

            remoteLogger.wait(2);
            
            //
            // Attempt reconnection with slightly different proxy
            //
            try
            {
                logger.attachRemoteLogger(Ice.RemoteLoggerPrxHelper.uncheckedCast(myProxy.ice_oneway()), 
                                          messageTypes, categories, 4);
                test(false);
            }
            catch(Ice.RemoteLoggerAlreadyAttachedException ex)
            {
                // expected
            }
            
            com.destroy();
        }
        out.println("ok");
     
        out.print("testing custom facet... ");
        out.flush();
        {
            //
            // Test: Verify that the custom facet is present.
            //
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            TestFacetPrx tf = TestFacetPrxHelper.checkedCast(obj, "TestFacet");
            tf.op();
            com.destroy();
        }
        out.println("ok");

        out.print("testing facet filtering... ");
        out.flush();
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the Properties facet,
            // meaning no other facet is available.
            //
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Properties");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.ProcessPrx proc = Ice.ProcessPrxHelper.checkedCast(obj, "Process");
            test(proc == null);
            TestFacetPrx tf = TestFacetPrxHelper.checkedCast(obj, "TestFacet");
            test(tf == null);
            com.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the Process facet,
            // meaning no other facet is available.
            //
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Process");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
            test(pa == null);
            TestFacetPrx tf = TestFacetPrxHelper.checkedCast(obj, "TestFacet");
            test(tf == null);
            com.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
            // meaning no other facet is available.
            //
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "TestFacet");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
            test(pa == null);
            Ice.ProcessPrx proc = Ice.ProcessPrxHelper.checkedCast(obj, "Process");
            test(proc == null);
            com.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
            // facet names.
            //
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "Properties TestFacet");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
            test(pa.getProperty("Ice.Admin.InstanceName").equals("Test"));
            TestFacetPrx tf = TestFacetPrxHelper.checkedCast(obj, "TestFacet");
            tf.op();
            Ice.ProcessPrx proc = Ice.ProcessPrxHelper.checkedCast(obj, "Process");
            test(proc == null);
            com.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
            // facet names.
            //
            java.util.Map<String, String> props = new java.util.HashMap<String, String>();
            props.put("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.put("Ice.Admin.InstanceName", "Test");
            props.put("Ice.Admin.Facets", "TestFacet, Process");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
            test(pa == null);
            TestFacetPrx tf = TestFacetPrxHelper.checkedCast(obj, "TestFacet");
            tf.op();
            Ice.ProcessPrx proc = Ice.ProcessPrxHelper.checkedCast(obj, "Process");
            proc.shutdown();
            com.waitForShutdown();
            com.destroy();
        }
        out.println("ok");

        factory.shutdown();
    }
}
