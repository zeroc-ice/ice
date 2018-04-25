// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

using Test;

public class AllTests : TestCommon.AllTests
{
    static void
    testFacets(Ice.Communicator com, bool builtInFacets)
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

        Dictionary<string, Ice.Object> facetMap = com.findAllAdminFacets();
        if(builtInFacets)
        {
            test(facetMap.Count == 7);
            test(facetMap.ContainsKey("Properties"));
            test(facetMap.ContainsKey("Process"));
            test(facetMap.ContainsKey("Logger"));
            test(facetMap.ContainsKey("Metrics"));
        }
        else
        {
            test(facetMap.Count >= 3);
        }
        test(facetMap.ContainsKey("Facet1"));
        test(facetMap.ContainsKey("Facet2"));
        test(facetMap.ContainsKey("Facet3"));

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

    public static void allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
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
            testFacets(com, true);
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
            // Test: Verify that the operations work correctly with Ice.Admin.Enabled=1
            //
            Ice.InitializationData init = new Ice.InitializationData();
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
            catch(Ice.InitializationException)
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
            Ice.InitializationData init = new Ice.InitializationData();
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
        WriteLine("ok");

        string @ref = "factory:" + app.getTestEndpoint(0) + " -t 10000";
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

        Write("testing logger facet... ");
        Flush();
        {
            Dictionary<String, String> props = new Dictionary<String, String>();
            props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            props.Add("Ice.Admin.InstanceName", "Test");
            props.Add("NullLogger", "1");
            RemoteCommunicatorPrx com = factory.createCommunicator(props);

            com.trace("testCat", "trace");
            com.warning("warning");
            com.error("error");
            com.print("print");

            Ice.ObjectPrx obj = com.getAdmin();
            Ice.LoggerAdminPrx logger = Ice.LoggerAdminPrxHelper.checkedCast(obj, "Logger");
            test(logger != null);

            string prefix = null;

            //
            // Get all
            //
            Ice.LogMessage[] logMessages = logger.getLog(null, null, -1, out prefix);

            test(logMessages.Length == 4);
            test(prefix.Equals("NullLogger"));
            test(logMessages[0].traceCategory.Equals("testCat") && logMessages[0].message.Equals("trace"));
            test(logMessages[1].message.Equals("warning"));
            test(logMessages[2].message.Equals("error"));
            test(logMessages[3].message.Equals("print"));

            //
            // Get only errors and warnings
            //
            com.error("error2");
            com.print("print2");
            com.trace("testCat", "trace2");
            com.warning("warning2");

            Ice.LogMessageType[] messageTypes = {Ice.LogMessageType.ErrorMessage, Ice.LogMessageType.WarningMessage};

            logMessages = logger.getLog(messageTypes, null, -1, out prefix);

            test(logMessages.Length == 4);
            test(prefix.Equals("NullLogger"));

            foreach(var msg in logMessages)
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
            string[] categories = {"testCat"};
            logMessages = logger.getLog(messageTypes, categories, -1, out prefix);
            test(logMessages.Length == 5);
            test(prefix.Equals("NullLogger"));

            foreach(var msg in logMessages)
            {
                test(msg.type == Ice.LogMessageType.ErrorMessage ||
                     (msg.type == Ice.LogMessageType.TraceMessage && msg.traceCategory.Equals("testCat")));
            }

            //
            // Same, but limited to last 2 messages (trace3 + error3)
            //
            com.error("error3");

            logMessages = logger.getLog(messageTypes, categories, 2, out prefix);
            test(logMessages.Length == 2);
            test(prefix.Equals("NullLogger"));

            test(logMessages[0].message.Equals("trace3"));
            test(logMessages[1].message.Equals("error3"));

            //
            // Now, test RemoteLogger
            //
            Ice.ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");

            RemoteLoggerI remoteLogger = new RemoteLoggerI();

            Ice.RemoteLoggerPrx myProxy = Ice.RemoteLoggerPrxHelper.uncheckedCast(adapter.addWithUUID(remoteLogger));

            adapter.activate();

            //
            // No filtering
            //
            logMessages = logger.getLog(null, null, -1, out prefix);
            remoteLogger.checkNextInit(prefix, logMessages);

            logger.attachRemoteLogger(myProxy, null, null, -1);
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
            logMessages = logger.getLog(messageTypes, categories, 4, out prefix);
            test(logMessages.Length == 4);
            remoteLogger.checkNextInit(prefix, logMessages);

            logger.attachRemoteLogger(myProxy, messageTypes, categories, 4);
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
            catch(Ice.RemoteLoggerAlreadyAttachedException)
            {
                // expected
            }

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

    private class RemoteLoggerI : Ice.RemoteLoggerDisp_
    {
        override public void init(string prefix, Ice.LogMessage[] logMessages, Ice.Current current)
        {
            lock(this)
            {
                test(prefix.Equals(_expectedPrefix));
                test(Enumerable.SequenceEqual(logMessages, _expectedInitMessages));
                _receivedCalls++;
                Monitor.PulseAll(this);
            }
        }

        override public void log(Ice.LogMessage logMessage, Ice.Current current)
        {
            lock(this)
            {
                Ice.LogMessage front = _expectedLogMessages.Dequeue();
                test(front.type == logMessage.type && front.message.Equals(logMessage.message) &&
                     front.traceCategory.Equals(logMessage.traceCategory));

                _receivedCalls++;
                Monitor.PulseAll(this);
            }
        }

        internal void checkNextInit(string prefix, Ice.LogMessage[] logMessages)
        {
            lock(this)
            {
                _expectedPrefix = prefix;
                _expectedInitMessages = logMessages;
            }
        }

        internal void checkNextLog(Ice.LogMessageType messageType,string message, string category)
        {
            lock(this)
            {
                Ice.LogMessage logMessage = new Ice.LogMessage(messageType, 0, category, message);
                _expectedLogMessages.Enqueue(logMessage);
            }
        }

        internal void wait(int calls)
        {
            lock(this)
            {
                _receivedCalls -= calls;

                while(_receivedCalls < 0)
                {
                    Monitor.Wait(this);
                }
            }
        }

        private int _receivedCalls = 0;
        private string _expectedPrefix;
        private Ice.LogMessage[] _expectedInitMessages;
        private readonly Queue<Ice.LogMessage> _expectedLogMessages = new Queue<Ice.LogMessage>();
    }
}
