//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading;
using Ice.admin.Test;

namespace Ice.admin
{
    public class AllTests : global::Test.AllTests
    {
        static void
        testFacets(Communicator com, bool builtInFacets, bool filtered)
        {
            if (builtInFacets && !filtered)
            {
                test(com.FindAdminFacet("Properties") != null);
                test(com.FindAdminFacet("Process") != null);
                test(com.FindAdminFacet("Logger") != null);
                test(com.FindAdminFacet("Metrics") != null);
            }

            var f1 = new TestFacet();
            var f2 = new TestFacet();
            var f3 = new TestFacet();

            if (!filtered)
            {
                com.AddAdminFacet("Facet1", f1);
                com.AddAdminFacet("Facet2", f2);
                com.AddAdminFacet("Facet3", f3);
            }
            else
            {
                try
                {
                    com.AddAdminFacet("Facet1", f1);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    com.AddAdminFacet("Facet2", f2);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    com.AddAdminFacet("Facet3", f3);
                    test(false);
                }
                catch (ArgumentException)
                {
                }
            }

            if (!filtered)
            {
                test(com.FindAdminFacet("Facet1") == f1);
                test(com.FindAdminFacet("Facet2") == f2);
                test(com.FindAdminFacet("Facet3") == f3);
            }
            else
            {
                test(com.FindAdminFacet("Facet1") == null);
                test(com.FindAdminFacet("Facet2") == null);
                test(com.FindAdminFacet("Facet3") == null);
            }
            test(com.FindAdminFacet("Bogus") == null);

            Dictionary<string, IObject> facetMap = com.FindAllAdminFacets();
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
                    com.AddAdminFacet("Facet1", f1);
                    test(false);
                }
                catch (ArgumentException)
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
                com.Destroy();
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
                com.Destroy();
            }
            {
                //
                // Test: Verify that the operations work correctly with the Admin object disabled.
                //
                var com = new Communicator();
                testFacets(com, false, false);
                com.Destroy();
            }
            {
                //
                // Test: Verify that the operations work correctly with Ice.Admin.Enabled=1
                //
                var properties = new Dictionary<string, string>() {
                    { "Ice.Admin.Enabled", "1" }
                };
                Communicator com = new Communicator(properties);
                test(com.GetAdmin() == null);
                Identity id = Identity.Parse("test-admin");
                try
                {
                    com.CreateAdmin(null, id);
                    test(false);
                }
                catch (InitializationException)
                {
                }

                ObjectAdapter adapter = com.CreateObjectAdapter("");
                test(com.CreateAdmin(adapter, id) != null);
                test(com.GetAdmin() != null);

                testFacets(com, true, false);
                com.Destroy();
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
                com.GetAdmin();
                testFacets(com, true, false);
                com.Destroy();
            }
            output.WriteLine("ok");

            var factory = IRemoteCommunicatorFactoryPrx.Parse($"factory:{helper.getTestEndpoint(0)} -t 10000",
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
                IProcessPrx proc = IProcessPrx.UncheckedCast(obj.Clone(facet: "Process"));
                proc.Shutdown();
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
                IPropertiesAdminPrx pa = IPropertiesAdminPrx.UncheckedCast(obj.Clone(facet: "Properties"));

                //
                // Test: PropertiesAdmin::getProperty()
                //
                test(pa.GetProperty("Prop2") == "2");
                test(pa.GetProperty("Bogus") == "");

                //
                // Test: PropertiesAdmin::getProperties()
                //
                Dictionary<string, string> pd = pa.GetPropertiesForPrefix("");
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
                pa.SetProperties(setProps);
                test(pa.GetProperty("Prop1").Equals("10"));
                test(pa.GetProperty("Prop2").Equals("20"));
                test(pa.GetProperty("Prop3").Equals(""));
                test(pa.GetProperty("Prop4").Equals("4"));
                test(pa.GetProperty("Prop5").Equals("5"));
                changes = com.getChanges();
                test(changes.Count == 5);
                test(changes["Prop1"].Equals("10"));
                test(changes["Prop2"].Equals("20"));
                test(changes["Prop3"].Equals(""));
                test(changes["Prop4"].Equals("4"));
                test(changes["Prop5"].Equals("5"));
                pa.SetProperties(setProps);
                changes = com.getChanges();
                test(changes.Count == 0);

                com.destroy();
            }
            output.WriteLine("ok");

            output.Write("testing logger facet... ");
            output.Flush();
            {
                var props = new Dictionary<string, string>();
                props.Add("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
                props.Add("Ice.Admin.InstanceName", "Test");
                props.Add("NullLogger", "1");
                var com = factory.createCommunicator(props);

                com.trace("testCat", "trace");
                com.warning("warning");
                com.error("error");
                com.print("print");

                IObjectPrx obj = com.getAdmin();
                var logger = ILoggerAdminPrx.UncheckedCast(obj.Clone(facet: "Logger"));
                test(logger != null);

                //
                // Get all
                //
                var (logMessages, prefix) = logger.GetLog(Array.Empty<LogMessageType>(), Array.Empty<string>(), -1);

                test(logMessages.Length == 4);
                test(prefix.Equals("NullLogger"));
                test(logMessages[0].TraceCategory.Equals("testCat") && logMessages[0].Message.Equals("trace"));
                test(logMessages[1].Message.Equals("warning"));
                test(logMessages[2].Message.Equals("error"));
                test(logMessages[3].Message.Equals("print"));

                //
                // Get only errors and warnings
                //
                com.error("error2");
                com.print("print2");
                com.trace("testCat", "trace2");
                com.warning("warning2");

                LogMessageType[] messageTypes = { LogMessageType.ErrorMessage, LogMessageType.WarningMessage };

                (logMessages, prefix) = logger.GetLog(messageTypes, Array.Empty<string>(), -1);

                test(logMessages.Length == 4);
                test(prefix.Equals("NullLogger"));

                foreach (LogMessage msg in logMessages)
                {
                    test(msg.Type == LogMessageType.ErrorMessage || msg.Type == LogMessageType.WarningMessage);
                }

                //
                // Get only errors and traces with Cat = "testCat"
                //
                com.trace("testCat2", "A");
                com.trace("testCat", "trace3");
                com.trace("testCat2", "B");

                messageTypes = new LogMessageType[] { LogMessageType.ErrorMessage, LogMessageType.TraceMessage };
                string[] categories = { "testCat" };
                (logMessages, prefix) = logger.GetLog(messageTypes, categories, -1);
                test(logMessages.Length == 5);
                test(prefix.Equals("NullLogger"));

                foreach (var msg in logMessages)
                {
                    test(msg.Type == LogMessageType.ErrorMessage ||
                        (msg.Type == LogMessageType.TraceMessage && msg.TraceCategory.Equals("testCat")));
                }

                //
                // Same, but limited to last 2 messages(trace3 + error3)
                //
                com.error("error3");

                (logMessages, prefix) = logger.GetLog(messageTypes, categories, 2);
                test(logMessages.Length == 2);
                test(prefix.Equals("NullLogger"));

                test(logMessages[0].Message.Equals("trace3"));
                test(logMessages[1].Message.Equals("error3"));

                //
                // Now, test RemoteLogger
                //
                ObjectAdapter adapter =
                    communicator.CreateObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");

                var remoteLogger = new RemoteLogger();

                IRemoteLoggerPrx myProxy = adapter.AddWithUUID(remoteLogger, IRemoteLoggerPrx.Factory);

                adapter.Activate();

                //
                // No filtering
                //
                (logMessages, prefix) = logger.GetLog(Array.Empty<LogMessageType>(), Array.Empty<string>(), -1);

                logger.AttachRemoteLogger(myProxy, Array.Empty<LogMessageType>(), Array.Empty<string>(), -1);
                remoteLogger.Wait(1);

                foreach (var m in logMessages)
                {
                    remoteLogger.CheckNextInit(prefix, m.Type, m.Message, m.TraceCategory);
                }

                com.trace("testCat", "rtrace");
                com.warning("rwarning");
                com.error("rerror");
                com.print("rprint");

                remoteLogger.Wait(4);

                remoteLogger.CheckNextLog(LogMessageType.TraceMessage, "rtrace", "testCat");
                remoteLogger.CheckNextLog(LogMessageType.WarningMessage, "rwarning", "");
                remoteLogger.CheckNextLog(LogMessageType.ErrorMessage, "rerror", "");
                remoteLogger.CheckNextLog(LogMessageType.PrintMessage, "rprint", "");

                test(logger.DetachRemoteLogger(myProxy));
                test(!logger.DetachRemoteLogger(myProxy));

                //
                // Use Error + Trace with "traceCat" filter with 4 limit
                //
                (logMessages, prefix) = logger.GetLog(messageTypes, categories, 4);
                test(logMessages.Length == 4);

                logger.AttachRemoteLogger(myProxy, messageTypes, categories, 4);
                remoteLogger.Wait(1);

                foreach (var m in logMessages)
                {
                    remoteLogger.CheckNextInit(prefix, m.Type, m.Message, m.TraceCategory);
                }

                com.warning("rwarning2");
                com.trace("testCat", "rtrace2");
                com.warning("rwarning3");
                com.error("rerror2");
                com.print("rprint2");

                remoteLogger.Wait(2);

                remoteLogger.CheckNextLog(LogMessageType.TraceMessage, "rtrace2", "testCat");
                remoteLogger.CheckNextLog(LogMessageType.ErrorMessage, "rerror2", "");

                //
                // Attempt reconnection with slightly different proxy
                //
                try
                {
                    logger.AttachRemoteLogger(myProxy.Clone(oneway: true), messageTypes, categories, 4);
                    test(false);
                }
                catch (RemoteLoggerAlreadyAttachedException)
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
                var tf = ITestFacetPrx.UncheckedCast(obj.Clone(facet: "TestFacet"));
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
                    IProcessPrx.CheckedCast(obj.Clone(facet: "Process"));
                    test(false);
                }
                catch (ObjectNotExistException)
                {
                }

                try
                {
                    ITestFacetPrx.CheckedCast(obj.Clone(facet: "TestFacet"));
                    test(false);
                }
                catch (ObjectNotExistException)
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
                    IPropertiesAdminPrx.CheckedCast(obj.Clone(facet: "Properties"));
                    test(false);
                }
                catch (ObjectNotExistException)
                {
                }

                try
                {
                    ITestFacetPrx.CheckedCast(obj.Clone(facet: "TestFacet"));
                    test(false);
                }
                catch (ObjectNotExistException)
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
                    IPropertiesAdminPrx.CheckedCast(obj.Clone(facet: "Properties"));
                    test(false);
                }
                catch (ObjectNotExistException)
                {
                }

                try
                {
                    IProcessPrx.CheckedCast(obj.Clone(facet: "Process"));
                    test(false);
                }
                catch (ObjectNotExistException)
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
                IPropertiesAdminPrx pa = IPropertiesAdminPrx.UncheckedCast(obj.Clone(facet: "Properties"));
                test(pa.GetProperty("Ice.Admin.InstanceName").Equals("Test"));
                var tf = ITestFacetPrx.CheckedCast(obj.Clone(facet: "TestFacet"));
                tf!.op();
                try
                {
                    IProcessPrx.CheckedCast(obj.Clone(facet: "Process"));
                    test(false);
                }
                catch (ObjectNotExistException)
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
                    IPropertiesAdminPrx.CheckedCast(obj.Clone(facet: "Properties"));
                    test(false);
                }
                catch (ObjectNotExistException)
                {
                }
                var tf = ITestFacetPrx.CheckedCast(obj.Clone(facet: "TestFacet"));
                tf.op();
                IProcessPrx proc = IProcessPrx.CheckedCast(obj.Clone(facet: "Process"));
                proc.Shutdown();
                com.waitForShutdown();
                com.destroy();
            }
            output.WriteLine("ok");

            factory.shutdown();
        }

        private class RemoteLogger : IRemoteLogger
        {
            public void Init(string prefix, LogMessage[] messages, Current current)
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

            public void Log(LogMessage message, Current current)
            {
                lock (this)
                {
                    _logMessages.Enqueue(message);
                    _receivedCalls++;
                    Monitor.PulseAll(this);
                }
            }

            internal void CheckNextInit(string prefix, LogMessageType type, string message, string category)
            {
                lock (this)
                {
                    test(_prefix.Equals(prefix));
                    test(_initMessages.Count > 0);
                    var logMessage = _initMessages.Dequeue();
                    test(logMessage.Type == type);
                    test(logMessage.Message.Equals(message));
                    test(logMessage.TraceCategory.Equals(category));
                }
            }

            internal void CheckNextLog(LogMessageType type, string message, string category)
            {
                lock (this)
                {
                    test(_logMessages.Count > 0);
                    var logMessage = _logMessages.Dequeue();
                    test(logMessage.Type == type);
                    test(logMessage.Message.Equals(message));
                    test(logMessage.TraceCategory.Equals(category));
                }
            }

            internal void Wait(int calls)
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
            private readonly Queue<LogMessage> _initMessages = new Queue<LogMessage>();
            private readonly Queue<LogMessage> _logMessages = new Queue<LogMessage>();
        }
    }
}
