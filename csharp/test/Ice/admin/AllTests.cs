//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading;
using Ice.admin.Test;

namespace Ice
{
    namespace admin
    {
        public class AllTests : global::Test.AllTests
        {
            static void
            testFacets(Communicator com, bool builtInFacets, bool filtered)
            {
                if (builtInFacets && !filtered)
                {
                    test(com.FindAdminFacet("Properties").servant != null);
                    test(com.FindAdminFacet("Process").servant != null);
                    test(com.FindAdminFacet("Logger").servant != null);
                    test(com.FindAdminFacet("Metrics").servant != null);
                }

                var f1 = new TestFacetI();
                var f2 = new TestFacetI();
                var f3 = new TestFacetI();

                if (!filtered)
                {
                    com.AddAdminFacet<TestFacet, TestFacetTraits>(f1, "Facet1");
                    com.AddAdminFacet<TestFacet, TestFacetTraits>(f2, "Facet2");
                    com.AddAdminFacet<TestFacet, TestFacetTraits>(f3, "Facet3");
                }
                else
                {
                    try
                    {
                        com.AddAdminFacet<TestFacet, TestFacetTraits>(f1, "Facet1");
                        test(false);
                    }
                    catch (ArgumentException)
                    {
                    }

                    try
                    {
                        com.AddAdminFacet<TestFacet, TestFacetTraits>(f2, "Facet2");
                        test(false);
                    }
                    catch (ArgumentException)
                    {
                    }

                    try
                    {
                        com.AddAdminFacet<TestFacet, TestFacetTraits>(f3, "Facet3");
                        test(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                }

                if (!filtered)
                {
                    test(com.FindAdminFacet("Facet1").servant == f1);
                    test(com.FindAdminFacet("Facet2").servant == f2);
                    test(com.FindAdminFacet("Facet3").servant == f3);
                }
                else
                {
                    test(com.FindAdminFacet("Facet1").servant == null);
                    test(com.FindAdminFacet("Facet2").servant == null);
                    test(com.FindAdminFacet("Facet3").servant == null);
                }
                test(com.FindAdminFacet("Bogus").servant == null);

                Dictionary<string, (object servant, Disp disp)> facetMap = com.FindAllAdminFacets();
                if (builtInFacets)
                {
                    test(facetMap.Count == 7);
                    test(facetMap.ContainsKey("Properties"));
                    test(facetMap.ContainsKey("Process"));
                    test(facetMap.ContainsKey("Logger"));
                    test(facetMap.ContainsKey("Metrics"));
                }
                else if (filtered)
                {
                    test(facetMap.Count >= 1);
                    test(facetMap.ContainsKey("Properties"));
                }

                if (!filtered)
                {
                    test(facetMap.ContainsKey("Facet1"));
                    test(facetMap.ContainsKey("Facet2"));
                    test(facetMap.ContainsKey("Facet3"));

                    try
                    {
                        com.AddAdminFacet<TestFacet, TestFacetTraits>(f1, "Facet1");
                        test(false);
                    }
                    catch (AlreadyRegisteredException)
                    {
                        // Expected
                    }
                }

                try
                {
                    com.RemoveAdminFacet("Bogus");
                    test(false);
                }
                catch (NotRegisteredException)
                {
                    // Expected
                }

                if (!filtered)
                {
                    com.RemoveAdminFacet("Facet1");
                    com.RemoveAdminFacet("Facet2");
                    com.RemoveAdminFacet("Facet3");
                    try
                    {
                        com.RemoveAdminFacet("Facet1");
                        test(false);
                    }
                    catch (NotRegisteredException)
                    {
                        // Expected
                    }
                }
            }

            public static void allTests(global::Test.TestHelper helper)
            {
                Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing communicator operations... ");
                output.Flush();
                {
                    //
                    // Test: Exercise AddAdminFacet, FindAdminFacet, RemoveAdminFacet with a typical configuration.
                    //
                    var properties = new Dictionary<string, string>();
                    properties["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
                    properties["Ice.Admin.InstanceName"] = "Test";
                    var com = new Communicator(properties);
                    testFacets(com, true, false);
                    com.destroy();
                }
                {
                    //
                    // Test: Verify that the operations work correctly in the presence of facet filters.
                    //
                    var properties = new Dictionary<string, string>();
                    properties["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
                    properties["Ice.Admin.InstanceName"] = "Test";
                    properties["Ice.Admin.Facets"] = "Properties";
                    Communicator com = new Communicator(properties);
                    testFacets(com, false, true);
                    com.destroy();
                }
                {
                    //
                    // Test: Verify that the operations work correctly with the Admin object disabled.
                    //
                    var com = new Communicator();
                    testFacets(com, false, false);
                    com.destroy();
                }
                {
                    //
                    // Test: Verify that the operations work correctly with Ice.Admin.Enabled=1
                    //
                    var properties = new Dictionary<string, string>() {
                        { "Ice.Admin.Enabled", "1" }
                    };
                    Communicator com = new Communicator(properties);
                    test(com.getAdmin() == null);
                    Identity id = Identity.Parse("test-admin");
                    try
                    {
                        com.CreateAdmin(null, id);
                        test(false);
                    }
                    catch (InitializationException)
                    {
                    }

                    ObjectAdapter adapter = com.createObjectAdapter("");
                    test(com.CreateAdmin(adapter, id) != null);
                    test(com.getAdmin() != null);

                    testFacets(com, true, false);
                    com.destroy();
                }
                {
                    //
                    // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
                    //
                    var properties = new Dictionary<string, string>()
                    {
                        { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                        { "Ice.Admin.InstanceName", "Test" },
                        { "Ice.Admin.DelayCreation", "1" }
                    };

                    Communicator com = new Communicator(properties);
                    testFacets(com, true, false);
                    com.getAdmin();
                    testFacets(com, true, false);
                    com.destroy();
                }
                output.WriteLine("ok");

                var factory = RemoteCommunicatorFactoryPrx.Parse($"factory:{helper.getTestEndpoint(0)} -t 10000",
                    communicator);

                output.Write("testing process facet... ");
                output.Flush();
                {
                    //
                    // Test: Verify that Process::shutdown() operation shuts down the communicator.
                    //
                    Dictionary<string, string> props = new Dictionary<string, string>();
                    props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
                    props.Add("Ice.Admin.InstanceName", "Test");
                    var com = factory.createCommunicator(props);
                    IObjectPrx obj = com.getAdmin();
                    ProcessPrx proc = ProcessPrx.UncheckedCast(obj.Clone(facet: "Process"));
                    proc.shutdown();
                    com.waitForShutdown();
                    com.destroy();
                }
                output.WriteLine("ok");

                output.Write("testing properties facet... ");
                output.Flush();
                {
                    Dictionary<string, string> props = new Dictionary<string, string>();
                    props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
                    props.Add("Ice.Admin.InstanceName", "Test");
                    props.Add("Prop1", "1");
                    props.Add("Prop2", "2");
                    props.Add("Prop3", "3");
                    var com = factory.createCommunicator(props);
                    IObjectPrx obj = com.getAdmin();
                    PropertiesAdminPrx pa = PropertiesAdminPrx.UncheckedCast(obj.Clone(facet: "Properties"));

                    //
                    // Test: PropertiesAdmin::getProperty()
                    //
                    test(pa.getProperty("Prop2") == "2");
                    test(pa.getProperty("Bogus") == "");

                    //
                    // Test: PropertiesAdmin::getProperties()
                    //
                    Dictionary<string, string> pd = pa.getPropertiesForPrefix("");
                    test(pd.Count == 6);
                    test(pd["Ice.ProgramName"] == "server");
                    test(pd["Ice.Admin.Endpoints"] == "tcp -h 127.0.0.1");
                    test(pd["Ice.Admin.InstanceName"] == "Test");
                    test(pd["Prop1"] == "1");
                    test(pd["Prop2"] == "2");
                    test(pd["Prop3"] == "3");

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
                output.WriteLine("ok");

                output.Write("testing logger facet... ");
                output.Flush();
                {
                    Dictionary<string, string> props = new Dictionary<string, string>();
                    props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
                    props.Add("Ice.Admin.InstanceName", "Test");
                    props.Add("NullLogger", "1");
                    var com = factory.createCommunicator(props);

                    com.trace("testCat", "trace");
                    com.warning("warning");
                    com.error("error");
                    com.print("print");

                    IObjectPrx obj = com.getAdmin();
                    LoggerAdminPrx logger = LoggerAdminPrx.UncheckedCast(obj.Clone(facet: "Logger"));
                    test(logger != null);

                    string prefix = null;

                    //
                    // Get all
                    //
                    LogMessage[] logMessages = logger.getLog(null, null, -1, out prefix);

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

                    LogMessageType[] messageTypes = { LogMessageType.ErrorMessage, LogMessageType.WarningMessage };

                    logMessages = logger.getLog(messageTypes, null, -1, out prefix);

                    test(logMessages.Length == 4);
                    test(prefix.Equals("NullLogger"));

                    foreach (var msg in logMessages)
                    {
                        test(msg.type == LogMessageType.ErrorMessage || msg.type == LogMessageType.WarningMessage);
                    }

                    //
                    // Get only errors and traces with Cat = "testCat"
                    //
                    com.trace("testCat2", "A");
                    com.trace("testCat", "trace3");
                    com.trace("testCat2", "B");

                    messageTypes = new LogMessageType[] { LogMessageType.ErrorMessage, LogMessageType.TraceMessage };
                    string[] categories = { "testCat" };
                    logMessages = logger.getLog(messageTypes, categories, -1, out prefix);
                    test(logMessages.Length == 5);
                    test(prefix.Equals("NullLogger"));

                    foreach (var msg in logMessages)
                    {
                        test(msg.type == LogMessageType.ErrorMessage ||
                            (msg.type == LogMessageType.TraceMessage && msg.traceCategory.Equals("testCat")));
                    }

                    //
                    // Same, but limited to last 2 messages(trace3 + error3)
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
                    ObjectAdapter adapter =
                        communicator.createObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");

                    RemoteLoggerI remoteLogger = new RemoteLoggerI();

                    RemoteLoggerPrx myProxy = adapter.Add(remoteLogger);

                    adapter.Activate();

                    //
                    // No filtering
                    //
                    logMessages = logger.getLog(null, null, -1, out prefix);

                    logger.attachRemoteLogger(myProxy, null, null, -1);
                    remoteLogger.wait(1);

                    foreach (var m in logMessages)
                    {
                        remoteLogger.checkNextInit(prefix, m.type, m.message, m.traceCategory);
                    }

                    com.trace("testCat", "rtrace");
                    com.warning("rwarning");
                    com.error("rerror");
                    com.print("rprint");

                    remoteLogger.wait(4);

                    remoteLogger.checkNextLog(Ice.LogMessageType.TraceMessage, "rtrace", "testCat");
                    remoteLogger.checkNextLog(Ice.LogMessageType.WarningMessage, "rwarning", "");
                    remoteLogger.checkNextLog(Ice.LogMessageType.ErrorMessage, "rerror", "");
                    remoteLogger.checkNextLog(Ice.LogMessageType.PrintMessage, "rprint", "");

                    test(logger.detachRemoteLogger(myProxy));
                    test(!logger.detachRemoteLogger(myProxy));

                    //
                    // Use Error + Trace with "traceCat" filter with 4 limit
                    //
                    logMessages = logger.getLog(messageTypes, categories, 4, out prefix);
                    test(logMessages.Length == 4);

                    logger.attachRemoteLogger(myProxy, messageTypes, categories, 4);
                    remoteLogger.wait(1);

                    foreach (var m in logMessages)
                    {
                        remoteLogger.checkNextInit(prefix, m.type, m.message, m.traceCategory);
                    }

                    com.warning("rwarning2");
                    com.trace("testCat", "rtrace2");
                    com.warning("rwarning3");
                    com.error("rerror2");
                    com.print("rprint2");

                    remoteLogger.wait(2);

                    remoteLogger.checkNextLog(Ice.LogMessageType.TraceMessage, "rtrace2", "testCat");
                    remoteLogger.checkNextLog(Ice.LogMessageType.ErrorMessage, "rerror2", "");

                    //
                    // Attempt reconnection with slightly different proxy
                    //
                    try
                    {
                        logger.attachRemoteLogger(myProxy.Clone(oneway: true), messageTypes, categories, 4);
                        test(false);
                    }
                    catch (Ice.RemoteLoggerAlreadyAttachedException)
                    {
                        // expected
                    }

                    com.destroy();
                }
                output.WriteLine("ok");

                output.Write("testing custom facet... ");
                output.Flush();
                {
                    //
                    // Test: Verify that the custom facet is present.
                    //
                    Dictionary<string, string> props = new Dictionary<string, string>();
                    props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
                    props.Add("Ice.Admin.InstanceName", "Test");
                    var com = factory.createCommunicator(props);
                    IObjectPrx obj = com.getAdmin();
                    var tf = TestFacetPrx.UncheckedCast(obj.Clone(facet: "TestFacet"));
                    tf.op();
                    com.destroy();
                }
                output.WriteLine("ok");

                output.Write("testing facet filtering... ");
                output.Flush();
                {
                    //
                    // Test: Set Ice.Admin.Facets to expose only the Properties facet,
                    // meaning no other facet is available.
                    //
                    Dictionary<string, string> props = new Dictionary<string, string>();
                    props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
                    props.Add("Ice.Admin.InstanceName", "Test");
                    props.Add("Ice.Admin.Facets", "Properties");
                    var com = factory.createCommunicator(props);
                    IObjectPrx obj = com.getAdmin();
                    try
                    {
                        ProcessPrx.CheckedCast(obj.Clone(facet: "Process"));
                        test(false);
                    }
                    catch (FacetNotExistException)
                    {
                    }

                    try
                    {
                        TestFacetPrx.CheckedCast(obj.Clone(facet: "TestFacet"));
                        test(false);
                    }
                    catch (FacetNotExistException)
                    {
                    }
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
                    var com = factory.createCommunicator(props);
                    IObjectPrx obj = com.getAdmin();
                    try
                    {
                        PropertiesAdminPrx.CheckedCast(obj.Clone(facet: "Properties"));
                        test(false);
                    }
                    catch (FacetNotExistException)
                    {
                    }

                    try
                    {
                        TestFacetPrx.CheckedCast(obj.Clone(facet: "TestFacet"));
                        test(false);
                    }
                    catch (FacetNotExistException)
                    {
                    }
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
                    var com = factory.createCommunicator(props);
                    IObjectPrx obj = com.getAdmin();
                    try
                    {
                        PropertiesAdminPrx.CheckedCast(obj.Clone(facet: "Properties"));
                        test(false);
                    }
                    catch (FacetNotExistException)
                    {
                    }

                    try
                    {
                        ProcessPrx.CheckedCast(obj.Clone(facet: "Process"));
                        test(false);
                    }
                    catch (FacetNotExistException)
                    {
                    }
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
                    var com = factory.createCommunicator(props);
                    IObjectPrx obj = com.getAdmin();
                    PropertiesAdminPrx pa = PropertiesAdminPrx.UncheckedCast(obj.Clone(facet: "Properties"));
                    test(pa.getProperty("Ice.Admin.InstanceName").Equals("Test"));
                    var tf = TestFacetPrx.CheckedCast(obj.Clone(facet: "TestFacet"));
                    tf!.op();
                    try
                    {
                        ProcessPrx.CheckedCast(obj.Clone(facet: "Process"));
                        test(false);
                    }
                    catch (FacetNotExistException)
                    {
                    }
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
                    var com = factory.createCommunicator(props);
                    IObjectPrx obj = com.getAdmin();
                    try
                    {
                        PropertiesAdminPrx.CheckedCast(obj.Clone(facet: "Properties"));
                        test(false);
                    }
                    catch (FacetNotExistException)
                    {
                    }
                    var tf = Test.TestFacetPrx.CheckedCast(obj.Clone(facet: "TestFacet"));
                    tf.op();
                    ProcessPrx proc = Ice.ProcessPrx.CheckedCast(obj.Clone(facet: "Process"));
                    proc.shutdown();
                    com.waitForShutdown();
                    com.destroy();
                }
                output.WriteLine("ok");

                factory.shutdown();
            }

            private class RemoteLoggerI : Ice.RemoteLogger
            {
                public void init(string prefix, Ice.LogMessage[] messages, Ice.Current current)
                {
                    lock (this)
                    {
                        _prefix = prefix;
                        foreach (var message in messages)
                        {
                            _initMessages.Enqueue(message);
                        }
                        _receivedCalls++;
                        Monitor.PulseAll(this);
                    }
                }

                public void log(Ice.LogMessage message, Ice.Current current)
                {
                    lock (this)
                    {
                        _logMessages.Enqueue(message);
                        _receivedCalls++;
                        Monitor.PulseAll(this);
                    }
                }

                internal void checkNextInit(string prefix, Ice.LogMessageType type, string message, string category)
                {
                    lock (this)
                    {
                        test(_prefix.Equals(prefix));
                        test(_initMessages.Count > 0);
                        var logMessage = _initMessages.Dequeue();
                        test(logMessage.type == type);
                        test(logMessage.message.Equals(message));
                        test(logMessage.traceCategory.Equals(category));
                    }
                }

                internal void checkNextLog(Ice.LogMessageType type, string message, string category)
                {
                    lock (this)
                    {
                        test(_logMessages.Count > 0);
                        var logMessage = _logMessages.Dequeue();
                        test(logMessage.type == type);
                        test(logMessage.message.Equals(message));
                        test(logMessage.traceCategory.Equals(category));
                    }
                }

                internal void wait(int calls)
                {
                    lock (this)
                    {
                        _receivedCalls -= calls;

                        while (_receivedCalls < 0)
                        {
                            Monitor.Wait(this);
                        }
                    }
                }

                private int _receivedCalls = 0;
                private string _prefix;
                private readonly Queue<Ice.LogMessage> _initMessages = new Queue<Ice.LogMessage>();
                private readonly Queue<Ice.LogMessage> _logMessages = new Queue<Ice.LogMessage>();
            }
        }
    }
}
