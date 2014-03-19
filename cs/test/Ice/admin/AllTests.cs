// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;

#if SILVERLIGHT
using System.Windows.Controls;
#endif

using Test;

public class AllTests : TestCommon.TestApp
{
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
        catch(Ice.AlreadyRegisteredException)
        {
            // Expected
        }

        try
        {
            com.removeAdminFacet("Bogus");
            test(false);
        }
        catch(Ice.NotRegisteredException)
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
        catch(Ice.NotRegisteredException)
        {
            // Expected
        }
    }

#if SILVERLIGHT
    override
    public void run(Ice.Communicator communicator)
#else
    public static void allTests(Ice.Communicator communicator)
#endif
    {
        Write("testing communicator operations... ");
        Flush();
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
        WriteLine("ok");

        string @ref = "factory:default -p 12010 -t 10000";
        RemoteCommunicatorFactoryPrx factory =
            RemoteCommunicatorFactoryPrxHelper.uncheckedCast(communicator.stringToProxy(@ref));

        Write("testing process facet... ");
        Flush();
        {
            //
            // Test: Verify that Process::shutdown() operation shuts down the communicator.
            //
            Dictionary<string, string> props = new Dictionary<string, string>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.ProcessPrx proc = Ice.ProcessPrxHelper.checkedCast(obj, "Process");
            proc.shutdown();
            com.waitForShutdown();
            com.destroy();
        }
        WriteLine("ok");

        Write("testing properties facet... ");
        Flush();
        {
            Dictionary<string, string> props = new Dictionary<string, string>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            props.Add("Prop1", "1");
            props.Add("Prop2", "2");
            props.Add("Prop3", "3");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");

            //
            // Test: PropertiesAdmin::getProperty()
            //
            test(pa.getProperty("Prop2").Equals("2"));
            test(pa.getProperty("Bogus").Equals(""));

            //
            // Test: PropertiesAdmin::getProperties()
            //
            Dictionary<string, string> pd = pa.getPropertiesForPrefix("");
            test(pd.Count == 5);
            test(pd["Ice.Admin.Endpoints"].Equals("tcp -h 127.0.0.1"));
            test(pd["Ice.Admin.InstanceName"].Equals("Test"));
            test(pd["Prop1"].Equals("1"));
            test(pd["Prop2"].Equals("2"));
            test(pd["Prop3"].Equals("3"));

            Dictionary<string, string> changes;

            //
            // Test: PropertiesAdmin::setProperties()
            //
            Dictionary<string, string> setProps = new Dictionary<string, string>();
            setProps.Add("Prop1", "10"); // Changed
            setProps.Add("Prop2", "20"); // Changed
            setProps.Add("Prop3", ""); // Removed
            setProps.Add("Prop4", "4"); // Added
            setProps.Add("Prop5", "5"); // Added
            pa.setProperties(setProps);
            test(pa.getProperty("Prop1").Equals("10"));
            test(pa.getProperty("Prop2").Equals("20"));
            test(pa.getProperty("Prop3").Equals(""));
            test(pa.getProperty("Prop4").Equals("4"));
            test(pa.getProperty("Prop5").Equals("5"));
            changes = com.getChanges();
            test(changes.Count == 5);
            test(changes["Prop1"].Equals("10"));
            test(changes["Prop2"].Equals("20"));
            test(changes["Prop3"].Equals(""));
            test(changes["Prop4"].Equals("4"));
            test(changes["Prop5"].Equals("5"));
            pa.setProperties(setProps);
            changes = com.getChanges();
            test(changes.Count == 0);

            com.destroy();
        }
        WriteLine("ok");

        Write("testing custom facet... ");
        Flush();
        {
            //
            // Test: Verify that the custom facet is present.
            //
            Dictionary<string, string> props = new Dictionary<string, string>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            TestFacetPrx tf = TestFacetPrxHelper.checkedCast(obj, "TestFacet");
            tf.op();
            com.destroy();
        }
        WriteLine("ok");

        Write("testing facet filtering... ");
        Flush();
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the Properties facet,
            // meaning no other facet is available.
            //
            Dictionary<string, string> props = new Dictionary<string, string>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            props.Add("Ice.Admin.Facets", "Properties");
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
            Dictionary<string, string> props = new Dictionary<string, string>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            props.Add("Ice.Admin.Facets", "Process");
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
            Dictionary<string, string> props = new Dictionary<string, string>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            props.Add("Ice.Admin.Facets", "TestFacet");
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
            Dictionary<string, string> props = new Dictionary<string, string>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            props.Add("Ice.Admin.Facets", "Properties TestFacet");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
            test(pa.getProperty("Ice.Admin.InstanceName").Equals("Test"));
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
            Dictionary<string, string> props = new Dictionary<string, string>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            props.Add("Ice.Admin.Facets", "TestFacet, Process");
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
        WriteLine("ok");

        factory.shutdown();
    }
}
