// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    testFacets(Ice.Communicator com)
    {
        test(com.findAdminFacet("Properties") != null);
        test(com.findAdminFacet("Process") != null);

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
            Ice.InitializationData init = new Ice.InitializationData();
            init.properties = Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            Ice.Communicator com = Ice.Util.initialize(init);
            testFacets(com);
            com.destroy();
        }
        {
            //
            // Test: Verify that the operations work correctly in the presence of facet filters.
            //
            Ice.InitializationData init = new Ice.InitializationData();
            init.properties = Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.Facets", "Properties");
            Ice.Communicator com = Ice.Util.initialize(init);
            testFacets(com);
            com.destroy();
        }
        {
            //
            // Test: Verify that the operations work correctly with the Admin object disabled.
            //
            Ice.Communicator com = Ice.Util.initialize();
            testFacets(com);
            com.destroy();
        }
        {
            //
            // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
            //
            Ice.InitializationData init = new Ice.InitializationData();
            init.properties = Ice.Util.createProperties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.DelayCreation", "1");
            Ice.Communicator com = Ice.Util.initialize(init);
            testFacets(com);
            com.getAdmin();
            testFacets(com);
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
