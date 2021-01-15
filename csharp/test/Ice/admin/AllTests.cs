// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Admin
{
    public static class AllTests
    {
        public static void TestFacets(Communicator com, bool builtInFacets, bool filtered)
        {
            if (builtInFacets && !filtered)
            {
                TestHelper.Assert(com.FindAdminFacet("Properties") != null);
                TestHelper.Assert(com.FindAdminFacet("Process") != null);
                TestHelper.Assert(com.FindAdminFacet("Logger") != null);
                TestHelper.Assert(com.FindAdminFacet("Metrics") != null);
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
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    com.AddAdminFacet("Facet2", f2);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    com.AddAdminFacet("Facet3", f3);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                }
            }

            if (!filtered)
            {
                TestHelper.Assert(com.FindAdminFacet("Facet1") == f1);
                TestHelper.Assert(com.FindAdminFacet("Facet2") == f2);
                TestHelper.Assert(com.FindAdminFacet("Facet3") == f3);
            }
            else
            {
                TestHelper.Assert(com.FindAdminFacet("Facet1") == null);
                TestHelper.Assert(com.FindAdminFacet("Facet2") == null);
                TestHelper.Assert(com.FindAdminFacet("Facet3") == null);
            }
            TestHelper.Assert(com.FindAdminFacet("Bogus") == null);

            IReadOnlyDictionary<string, IObject> facetMap = com.FindAllAdminFacets();
            if (builtInFacets)
            {
                TestHelper.Assert(facetMap.Count == 7);
                TestHelper.Assert(facetMap.ContainsKey("Properties"));
                TestHelper.Assert(facetMap.ContainsKey("Process"));
                TestHelper.Assert(facetMap.ContainsKey("Logger"));
                TestHelper.Assert(facetMap.ContainsKey("Metrics"));
            }
            else if (filtered)
            {
                TestHelper.Assert(facetMap.Count >= 1);
                TestHelper.Assert(facetMap.ContainsKey("Properties"));
            }

            if (!filtered)
            {
                TestHelper.Assert(facetMap.ContainsKey("Facet1"));
                TestHelper.Assert(facetMap.ContainsKey("Facet2"));
                TestHelper.Assert(facetMap.ContainsKey("Facet3"));

                try
                {
                    com.AddAdminFacet("Facet1", f1);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                    // Expected
                }
            }

            IObject? facet = com.RemoveAdminFacet("Bogus");
            TestHelper.Assert(facet == null);

            if (!filtered)
            {
                facet = com.RemoveAdminFacet("Facet1");
                TestHelper.Assert(facet == f1);
                com.RemoveAdminFacet("Facet2");
                com.RemoveAdminFacet("Facet3");
                facet = com.RemoveAdminFacet("Facet1");
                TestHelper.Assert(facet == null);
            }
        }

        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            TextWriter output = helper.Output;
            bool ice1 = helper.Protocol == Protocol.Ice1;

            output.Write("testing communicator operations... ");
            output.Flush();
            {
                // Test: Exercise AddAdminFacet, FindAdminFacet, RemoveAdminFacet with a typical configuration.
                var properties = new Dictionary<string, string>
                {
                    ["Ice.Admin.Endpoints"] = ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0",
                    ["Ice.ServerName"] = "127.0.0.1",
                    ["Ice.Admin.InstanceName"] = "Test"
                };
                await using var com = new Communicator(properties);
                await com.ActivateAsync();
                TestFacets(com, true, false);
            }
            {
                // Test: Verify that the operations work correctly in the presence of facet filters.
                var properties = new Dictionary<string, string>
                {
                    ["Ice.Admin.Endpoints"] = ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0",
                    ["Ice.ServerName"] = "127.0.0.1",
                    ["Ice.Admin.InstanceName"] = "Test",
                    ["Ice.Admin.Facets"] = "Properties"
                };
                await using var com = new Communicator(properties);
                await com.ActivateAsync();
                TestFacets(com, false, true);
            }
            {
                // Test: Verify that the operations work correctly with the Admin object disabled.
                await using var com = new Communicator();
                await com.ActivateAsync();
                TestFacets(com, false, false);
            }
            {
                // Test: Verify that the operations work correctly with Ice.Admin.Enabled=1
                var properties = new Dictionary<string, string>()
                {
                    { "Ice.Admin.Enabled", "1" }
                };
                await using var com = new Communicator(properties);
                await com.ActivateAsync();
                TestHelper.Assert(await com.GetAdminAsync() == null);
                var id = Identity.Parse("test-admin");
                try
                {
                    _ = await com.CreateAdminAsync(null, id);
                    TestHelper.Assert(false);
                }
                catch (InvalidConfigurationException)
                {
                }

                ObjectAdapter adapter = com.CreateObjectAdapter();
                TestHelper.Assert(await com.CreateAdminAsync(adapter, id) != null);
                TestHelper.Assert(await com.GetAdminAsync() != null);

                TestFacets(com, true, false);
            }
            {
                // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
                var properties = new Dictionary<string, string>()
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1" },
                    { "Ice.Admin.InstanceName", "Test" }
                };

                await using var com = new Communicator(properties);
                TestFacets(com, true, false);
                await com.ActivateAsync();
                TestFacets(com, true, false);
            }
            output.WriteLine("ok");

            var factory = IRemoteCommunicatorFactoryPrx.Parse(helper.GetTestProxy("factory", 0), communicator);

            output.Write("testing process facet... ");
            output.Flush();
            {
                // Test: Verify that Process.Shutdown() operation shuts down the communicator.
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1" },
                    { "Ice.Admin.InstanceName", "Test" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                IProcessPrx proc = obj.Clone(IProcessPrx.Factory, facet: "Process");
                proc.Shutdown();
                com.Shutdown();
                com.Destroy();
            }
            output.WriteLine("ok");

            output.Write("testing properties facet... ");
            output.Flush();
            {
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1" },
                    { "Ice.Admin.InstanceName", "Test" },
                    { "Prop1", "1" },
                    { "Prop2", "2" },
                    { "Prop3", "3" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                IPropertiesAdminPrx pa = obj.Clone(IPropertiesAdminPrx.Factory, facet: "Properties");

                // Test: PropertiesAdmin.GetProperty()
                TestHelper.Assert(pa.GetProperty("Prop2") == "2");
                TestHelper.Assert(pa.GetProperty("Bogus").Length == 0);

                // Test: PropertiesAdmin.GetProperties()
                Dictionary<string, string> pd = pa.GetPropertiesForPrefix("");
                TestHelper.Assert(pd.Count == 7);
                TestHelper.Assert(pd["Ice.ProgramName"] == "server");
                TestHelper.Assert(pd["Ice.Admin.Endpoints"] == (ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0"));
                TestHelper.Assert(pd["Ice.Admin.InstanceName"] == "Test");
                TestHelper.Assert(pd["Prop1"] == "1");
                TestHelper.Assert(pd["Prop2"] == "2");
                TestHelper.Assert(pd["Prop3"] == "3");

                Dictionary<string, string> changes;

                // Test: PropertiesAdmin.SetProperties()
                var setProps = new Dictionary<string, string>
                {
                    { "Prop1", "10" }, // Changed
                    { "Prop2", "20" }, // Changed
                    { "Prop3", "" }, // Removed
                    { "Prop4", "4" }, // Added
                    { "Prop5", "5" } // Added
                };
                pa.SetProperties(setProps);
                TestHelper.Assert(pa.GetProperty("Prop1").Equals("10"));
                TestHelper.Assert(pa.GetProperty("Prop2").Equals("20"));
                TestHelper.Assert(pa.GetProperty("Prop3").Length == 0);
                TestHelper.Assert(pa.GetProperty("Prop4").Equals("4"));
                TestHelper.Assert(pa.GetProperty("Prop5").Equals("5"));
                changes = com.GetChanges();
                TestHelper.Assert(changes.Count == 5);
                TestHelper.Assert(changes["Prop1"].Equals("10"));
                TestHelper.Assert(changes["Prop2"].Equals("20"));
                TestHelper.Assert(changes["Prop3"].Length == 0);
                TestHelper.Assert(changes["Prop4"].Equals("4"));
                TestHelper.Assert(changes["Prop5"].Equals("5"));
                pa.SetProperties(setProps);
                changes = com.GetChanges();
                TestHelper.Assert(changes.Count == 0);

                com.Destroy();
            }
            output.WriteLine("ok");

            output.Write("testing logger facet... ");
            output.Flush();
            {
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1" },
                    { "Ice.Admin.InstanceName", "Test" },
                    { "NullLogger", "1" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                com.Trace("testCat", "trace");
                com.Warning("warning");
                com.Error("error");
                com.Print("print");

                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                ILoggerAdminPrx logger = obj.Clone(ILoggerAdminPrx.Factory, facet: "Logger");

                // Get all
                (LogMessage[] logMessages, string prefix) =
                    logger.GetLog(Array.Empty<LogMessageType>(), Array.Empty<string>(), -1);

                TestHelper.Assert(logMessages.Length == 4);
                TestHelper.Assert(prefix.Equals("NullLogger"));
                TestHelper.Assert(logMessages[0].TraceCategory.Equals("testCat") && logMessages[0].Message.Equals("trace"));
                TestHelper.Assert(logMessages[1].Message.Equals("warning"));
                TestHelper.Assert(logMessages[2].Message.Equals("error"));
                TestHelper.Assert(logMessages[3].Message.Equals("print"));

                // Get only errors and warnings
                com.Error("error2");
                com.Print("print2");
                com.Trace("testCat", "trace2");
                com.Warning("warning2");

                LogMessageType[] messageTypes = { LogMessageType.ErrorMessage, LogMessageType.WarningMessage };

                (logMessages, prefix) = logger.GetLog(messageTypes, Array.Empty<string>(), -1);

                TestHelper.Assert(logMessages.Length == 4);
                TestHelper.Assert(prefix.Equals("NullLogger"));

                foreach (LogMessage msg in logMessages)
                {
                    TestHelper.Assert(msg.Type == LogMessageType.ErrorMessage || msg.Type == LogMessageType.WarningMessage);
                }

                // Get only errors and traces with Cat = "testCat"
                com.Trace("testCat2", "A");
                com.Trace("testCat", "trace3");
                com.Trace("testCat2", "B");

                messageTypes = new LogMessageType[] { LogMessageType.ErrorMessage, LogMessageType.TraceMessage };
                string[] categories = { "testCat" };
                (logMessages, prefix) = logger.GetLog(messageTypes, categories, -1);
                TestHelper.Assert(logMessages.Length == 5, $"logMessages.Length: {logMessages.Length}");
                TestHelper.Assert(prefix.Equals("NullLogger"));

                foreach (LogMessage msg in logMessages)
                {
                    TestHelper.Assert(msg.Type == LogMessageType.ErrorMessage ||
                        (msg.Type == LogMessageType.TraceMessage && msg.TraceCategory.Equals("testCat")));
                }

                // Same, but limited to last 2 messages(trace3 + error3)
                com.Error("error3");

                (logMessages, prefix) = logger.GetLog(messageTypes, categories, 2);
                TestHelper.Assert(logMessages.Length == 2);
                TestHelper.Assert(prefix.Equals("NullLogger"));

                TestHelper.Assert(logMessages[0].Message.Equals("trace3"));
                TestHelper.Assert(logMessages[1].Message.Equals("error3"));

                // Now, test RemoteLogger
                ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints("RemoteLoggerAdapter",
                    ice1 ? "tcp -h \"::0\"" : "ice+tcp://[::0]:0", serializeDispatch: true);

                var remoteLogger = new RemoteLogger();

                IRemoteLoggerPrx myProxy = adapter.AddWithUUID(remoteLogger, IRemoteLoggerPrx.Factory);

                await adapter.ActivateAsync();

                // No filtering
                (logMessages, prefix) = logger.GetLog(Array.Empty<LogMessageType>(), Array.Empty<string>(), -1);

                logger.AttachRemoteLogger(myProxy, Array.Empty<LogMessageType>(), Array.Empty<string>(), -1);
                remoteLogger.Wait(1);

                foreach (LogMessage m in logMessages)
                {
                    remoteLogger.CheckNextInit(prefix, m.Type, m.Message, m.TraceCategory);
                }

                com.Trace("testCat", "rtrace");
                com.Warning("rwarning");
                com.Error("rerror");
                com.Print("rprint");

                remoteLogger.Wait(4);

                remoteLogger.CheckNextLog(LogMessageType.TraceMessage, "rtrace", "testCat");
                remoteLogger.CheckNextLog(LogMessageType.WarningMessage, "rwarning", "");
                remoteLogger.CheckNextLog(LogMessageType.ErrorMessage, "rerror", "");
                remoteLogger.CheckNextLog(LogMessageType.PrintMessage, "rprint", "");

                TestHelper.Assert(logger.DetachRemoteLogger(myProxy));
                TestHelper.Assert(!logger.DetachRemoteLogger(myProxy));

                // Use Error + Trace with "traceCat" filter with 4 limit
                (logMessages, prefix) = logger.GetLog(messageTypes, categories, 4);
                TestHelper.Assert(logMessages.Length == 4);

                logger.AttachRemoteLogger(myProxy, messageTypes, categories, 4);
                remoteLogger.Wait(1);

                foreach (LogMessage m in logMessages)
                {
                    remoteLogger.CheckNextInit(prefix, m.Type, m.Message, m.TraceCategory);
                }

                com.Warning("rwarning2");
                com.Trace("testCat", "rtrace2");
                com.Warning("rwarning3");
                com.Error("rerror2");
                com.Print("rprint2");

                remoteLogger.Wait(2);

                remoteLogger.CheckNextLog(LogMessageType.TraceMessage, "rtrace2", "testCat");
                remoteLogger.CheckNextLog(LogMessageType.ErrorMessage, "rerror2", "");

                // Attempt reconnection with slightly different proxy
                try
                {
                    logger.AttachRemoteLogger(myProxy.Clone(oneway: true), messageTypes, categories, 4);
                    TestHelper.Assert(false);
                }
                catch (RemoteLoggerAlreadyAttachedException)
                {
                    // expected
                }

                com.Destroy();
            }
            output.WriteLine("ok");

            output.Write("testing custom facet... ");
            output.Flush();
            {
                // Test: Verify that the custom facet is present.
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1"},
                    { "Ice.Admin.InstanceName", "Test" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                ITestFacetPrx tf = obj.Clone(ITestFacetPrx.Factory, facet: "TestFacet");
                tf.Op();
                com.Destroy();
            }
            output.WriteLine("ok");

            output.Write("testing facet filtering... ");
            output.Flush();
            {
                // Test: Set Ice.Admin.Facets to expose only the Properties facet, meaning no other facet is available.
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1"},
                    { "Ice.Admin.InstanceName", "Test" },
                    { "Ice.Admin.Facets", "Properties" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                try
                {
                    obj.Clone(IProcessPrx.Factory, facet: "Process").IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }

                try
                {
                    obj.Clone(ITestFacetPrx.Factory, facet: "TestFacet").IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }
                com.Destroy();
            }
            {
                // Test: Set Ice.Admin.Facets to expose only the Process facet, meaning no other facet is available.
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1"},
                    { "Ice.Admin.InstanceName", "Test" },
                    { "Ice.Admin.Facets", "Process" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                try
                {
                    obj.Clone(IPropertiesAdminPrx.Factory, facet: "Properties").IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }

                try
                {
                    obj.Clone(ITestFacetPrx.Factory, facet: "TestFacet").IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }
                com.Destroy();
            }
            {
                // Test: Set Ice.Admin.Facets to expose only the TestFacet facet, meaning no other facet is available.
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1"},
                    { "Ice.Admin.InstanceName", "Test" },
                    { "Ice.Admin.Facets", "TestFacet" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                try
                {
                    obj.Clone(IPropertiesAdminPrx.Factory, facet: "Properties").IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }

                try
                {
                    obj.Clone(IProcessPrx.Factory, facet: "Process").IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }
                com.Destroy();
            }
            {
                // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the facet names.
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1"},
                    { "Ice.Admin.InstanceName", "Test" },
                    { "Ice.Admin.Facets", "Properties TestFacet" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                IPropertiesAdminPrx pa = obj.Clone(IPropertiesAdminPrx.Factory, facet: "Properties");
                TestHelper.Assert(pa.GetProperty("Ice.Admin.InstanceName").Equals("Test"));
                ITestFacetPrx? tf =
                    obj.Clone(IObjectPrx.Factory, facet: "TestFacet").CheckedCast(ITestFacetPrx.Factory);
                tf!.Op();
                try
                {
                    obj.Clone(IProcessPrx.Factory, facet: "Process").IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }
                com.Destroy();
            }
            {
                // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the facet names.
                var props = new Dictionary<string, string>
                {
                    { "Ice.Admin.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0" },
                    { "Ice.ServerName", "127.0.0.1"},
                    { "Ice.Admin.InstanceName", "Test" },
                    { "Ice.Admin.Facets", "TestFacet, Process" }
                };
                IRemoteCommunicatorPrx? com = factory.CreateCommunicator(props);
                TestHelper.Assert(com != null);
                IObjectPrx? obj = com.GetAdmin();
                TestHelper.Assert(obj != null);
                try
                {
                    obj.Clone(IPropertiesAdminPrx.Factory, facet: "Properties").IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }
                ITestFacetPrx? tf =
                    obj.Clone(IObjectPrx.Factory, facet: "TestFacet").CheckedCast(ITestFacetPrx.Factory);
                TestHelper.Assert(tf != null);
                tf.Op();
                IProcessPrx? proc =
                    obj.Clone(IObjectPrx.Factory, facet: "Process").CheckedCast(IProcessPrx.Factory);
                TestHelper.Assert(proc != null);
                proc.Shutdown();
                com.Shutdown();
                com.Destroy();
            }
            output.WriteLine("ok");

            factory.Shutdown();
        }

        private class RemoteLogger : IRemoteLogger
        {
            private readonly Queue<LogMessage> _initMessages = new();
            private readonly Queue<LogMessage> _logMessages = new();
            private readonly object _mutex = new();
            private string? _prefix;
            private int _receivedCalls;

            public void Init(string prefix, LogMessage[] messages, Current current, CancellationToken cancel)
            {
                lock (_mutex)
                {
                    _prefix = prefix;
                    foreach (LogMessage message in messages)
                    {
                        _initMessages.Enqueue(message);
                    }
                    _receivedCalls++;
                    Monitor.PulseAll(_mutex);
                }
            }

            public void Log(LogMessage message, Current current, CancellationToken cancel)
            {
                lock (_mutex)
                {
                    _logMessages.Enqueue(message);
                    _receivedCalls++;
                    Monitor.PulseAll(_mutex);
                }
            }

            internal void CheckNextInit(string prefix, LogMessageType type, string message, string category)
            {
                lock (_mutex)
                {
                    TestHelper.Assert(_prefix != null);
                    TestHelper.Assert(_prefix.Equals(prefix));
                    TestHelper.Assert(_initMessages.Count > 0);
                    LogMessage logMessage = _initMessages.Dequeue();
                    TestHelper.Assert(logMessage.Type == type);
                    TestHelper.Assert(logMessage.Message.Equals(message));
                    TestHelper.Assert(logMessage.TraceCategory.Equals(category));
                }
            }

            internal void CheckNextLog(LogMessageType type, string message, string category)
            {
                lock (_mutex)
                {
                    TestHelper.Assert(_logMessages.Count > 0);
                    LogMessage logMessage = _logMessages.Dequeue();
                    TestHelper.Assert(logMessage.Type == type);
                    TestHelper.Assert(logMessage.Message.Equals(message));
                    TestHelper.Assert(logMessage.TraceCategory.Equals(category));
                }
            }

            internal void Wait(int calls)
            {
                lock (_mutex)
                {
                    _receivedCalls -= calls;

                    while (_receivedCalls < 0)
                    {
                        Monitor.Wait(_mutex);
                    }
                }
            }
        }
    }
}
