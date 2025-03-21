// Copyright (c) ZeroC, Inc.

namespace Ice.admin;

public class AllTests : global::Test.AllTests
{
    private static void
    testFacets(Ice.Communicator com, bool builtInFacets)
    {
        if (builtInFacets)
        {
            test(com.findAdminFacet("Properties") != null);
            test(com.findAdminFacet("Process") != null);
            test(com.findAdminFacet("Logger") != null);
            test(com.findAdminFacet("Metrics") != null);
        }

        Test.TestFacet f1 = new TestFacetI();
        Test.TestFacet f2 = new TestFacetI();
        Test.TestFacet f3 = new TestFacetI();

        com.addAdminFacet(f1, "Facet1");
        com.addAdminFacet(f2, "Facet2");
        com.addAdminFacet(f3, "Facet3");

        test(com.findAdminFacet("Facet1") == f1);
        test(com.findAdminFacet("Facet2") == f2);
        test(com.findAdminFacet("Facet3") == f3);
        test(com.findAdminFacet("Bogus") == null);

        Dictionary<string, Ice.Object> facetMap = com.findAllAdminFacets();
        if (builtInFacets)
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
        catch (Ice.AlreadyRegisteredException)
        {
            // Expected
        }

        try
        {
            com.removeAdminFacet("Bogus");
            test(false);
        }
        catch (Ice.NotRegisteredException)
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
        catch (Ice.NotRegisteredException)
        {
            // Expected
        }
    }

    public static void allTests(global::Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        var output = helper.getWriter();
        output.Write("testing communicator operations... ");
        output.Flush();
        {
            //
            // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
            //
            Ice.InitializationData init = new Ice.InitializationData();
            init.properties = new Ice.Properties();
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
            init.properties = new Ice.Properties();
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
            init.properties = new Ice.Properties();
            init.properties.setProperty("Ice.Admin.Enabled", "1");
            Ice.Communicator com = Ice.Util.initialize(init);
            test(com.getAdmin() == null);
            Ice.Identity id = Ice.Util.stringToIdentity("test-admin");
            try
            {
                com.createAdmin(null, id);
                test(false);
            }
            catch (Ice.InitializationException)
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
            init.properties = new Ice.Properties();
            init.properties.setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
            init.properties.setProperty("Ice.Admin.InstanceName", "Test");
            init.properties.setProperty("Ice.Admin.DelayCreation", "1");
            Ice.Communicator com = Ice.Util.initialize(init);
            testFacets(com, true);
            com.getAdmin();
            testFacets(com, true);
            com.destroy();
        }
        output.WriteLine("ok");

        string @ref = "factory:" + helper.getTestEndpoint(0) + " -t 10000";
        Test.RemoteCommunicatorFactoryPrx factory =
            Test.RemoteCommunicatorFactoryPrxHelper.uncheckedCast(communicator.stringToProxy(@ref));

        output.Write("testing process facet... ");
        output.Flush();
        {
            //
            // Test: Verify that Process::shutdown() operation shuts down the communicator.
            //
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" }
            };
            var com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.ProcessPrx proc = Ice.ProcessPrxHelper.checkedCast(obj, "Process");
            proc.shutdown();
            com.waitForShutdown();
            com.destroy();
        }
        output.WriteLine("ok");

        output.Write("testing properties facet... ");
        output.Flush();
        {
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" },
                { "Prop1", "1" },
                { "Prop2", "2" },
                { "Prop3", "3" }
            };
            var com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");

            //
            // Test: PropertiesAdmin::getProperty()
            //
            test(pa.getProperty("Prop2") == "2");
            test(pa.getProperty("Bogus").Length == 0);

            //
            // Test: PropertiesAdmin::getProperties()
            //
            Dictionary<string, string> pd = pa.getPropertiesForPrefix("");
            test(pd.Count == 5);
            test(pd["Ice.Admin.Endpoints"] == "tcp -h 127.0.0.1");
            test(pd["Ice.Admin.InstanceName"] == "Test");
            test(pd["Prop1"] == "1");
            test(pd["Prop2"] == "2");
            test(pd["Prop3"] == "3");

            Dictionary<string, string> changes;

            //
            // Test: PropertiesAdmin::setProperties()
            //
            Dictionary<string, string> setProps = new Dictionary<string, string>
            {
                { "Prop1", "10" }, // Changed
                { "Prop2", "20" }, // Changed
                { "Prop3", "" }, // Removed
                { "Prop4", "4" }, // Added
                { "Prop5", "5" } // Added
            };
            pa.setProperties(setProps);
            test(pa.getProperty("Prop1") == "10");
            test(pa.getProperty("Prop2") == "20");
            test(pa.getProperty("Prop3").Length == 0);
            test(pa.getProperty("Prop4") == "4");
            test(pa.getProperty("Prop5") == "5");
            changes = com.getChanges();
            test(changes.Count == 5);
            test(changes["Prop1"] == "10");
            test(changes["Prop2"] == "20");
            test(changes["Prop3"].Length == 0);
            test(changes["Prop4"] == "4");
            test(changes["Prop5"] == "5");
            pa.setProperties(setProps);
            changes = com.getChanges();
            test(changes.Count == 0);

            com.destroy();
        }
        output.WriteLine("ok");

        output.Write("testing logger facet... ");
        output.Flush();
        {
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" },
                { "NullLogger", "1" }
            };
            var com = factory.createCommunicator(props);

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
            test(prefix == "NullLogger");
            test(logMessages[0].traceCategory == "testCat" && logMessages[0].message == "trace");
            test(logMessages[1].message == "warning");
            test(logMessages[2].message == "error");
            test(logMessages[3].message == "print");

            //
            // Get only errors and warnings
            //
            com.error("error2");
            com.print("print2");
            com.trace("testCat", "trace2");
            com.warning("warning2");

            Ice.LogMessageType[] messageTypes = {
                            Ice.LogMessageType.ErrorMessage,
                            Ice.LogMessageType.WarningMessage
                        };

            logMessages = logger.getLog(messageTypes, null, -1, out prefix);

            test(logMessages.Length == 4);
            test(prefix == "NullLogger");

            foreach (var msg in logMessages)
            {
                test(msg.type == Ice.LogMessageType.ErrorMessage ||
                     msg.type == Ice.LogMessageType.WarningMessage);
            }

            //
            // Get only errors and traces with Cat = "testCat"
            //
            com.trace("testCat2", "A");
            com.trace("testCat", "trace3");
            com.trace("testCat2", "B");

            messageTypes = new Ice.LogMessageType[] {
                            Ice.LogMessageType.ErrorMessage,
                            Ice.LogMessageType.TraceMessage
                        };
            string[] categories = { "testCat" };
            logMessages = logger.getLog(messageTypes, categories, -1, out prefix);
            test(logMessages.Length == 5);
            test(prefix == "NullLogger");

            foreach (var msg in logMessages)
            {
                test(msg.type == Ice.LogMessageType.ErrorMessage ||
                    (msg.type == Ice.LogMessageType.TraceMessage && msg.traceCategory == "testCat"));
            }

            //
            // Same, but limited to last 2 messages(trace3 + error3)
            //
            com.error("error3");

            logMessages = logger.getLog(messageTypes, categories, 2, out prefix);
            test(logMessages.Length == 2);
            test(prefix == "NullLogger");

            test(logMessages[0].message == "trace3");
            test(logMessages[1].message == "error3");

            //
            // Now, test RemoteLogger
            //
            Ice.ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");

            RemoteLoggerI remoteLogger = new RemoteLoggerI();

            Ice.RemoteLoggerPrx myProxy =
                Ice.RemoteLoggerPrxHelper.uncheckedCast(adapter.addWithUUID(remoteLogger));

            adapter.activate();

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
                logger.attachRemoteLogger(Ice.RemoteLoggerPrxHelper.uncheckedCast(myProxy.ice_oneway()),
                                          messageTypes, categories, 4);
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
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" }
            };
            var com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            var tf = Test.TestFacetPrxHelper.checkedCast(obj, "TestFacet");
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
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" },
                { "Ice.Admin.Facets", "Properties" }
            };
            var com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            try
            {
                ProcessPrxHelper.checkedCast(obj, "Process");
                test(false);
            }
            catch (FacetNotExistException)
            {
                // Expected
            }
            try
            {
                Test.TestFacetPrxHelper.checkedCast(obj, "TestFacet");
                test(false);
            }
            catch (FacetNotExistException)
            {
                // Expected
            }
            com.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the Process facet,
            // meaning no other facet is available.
            //
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" },
                { "Ice.Admin.Facets", "Process" }
            };
            var com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            try
            {
                PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
                test(false);
            }
            catch (FacetNotExistException)
            {
                // Expected
            }
            try
            {
                Test.TestFacetPrxHelper.checkedCast(obj, "TestFacet");
                test(false);
            }
            catch (FacetNotExistException)
            {
                // Expected
            }
            com.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
            // meaning no other facet is available.
            //
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" },
                { "Ice.Admin.Facets", "TestFacet" }
            };
            var com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            try
            {
                PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
                test(false);
            }
            catch (FacetNotExistException)
            {
                // Expected
            }
            try
            {
                ProcessPrxHelper.checkedCast(obj, "Process");
                test(false);
            }
            catch (FacetNotExistException)
            {
                // Expected
            }
            com.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
            // facet names.
            //
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" },
                { "Ice.Admin.Facets", "Properties TestFacet" }
            };
            var com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            Ice.PropertiesAdminPrx pa = Ice.PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
            test(pa.getProperty("Ice.Admin.InstanceName") == "Test");
            var tf = Test.TestFacetPrxHelper.checkedCast(obj, "TestFacet");
            tf.op();
            try
            {
                ProcessPrxHelper.checkedCast(obj, "Process");
                test(false);
            }
            catch (FacetNotExistException)
            {
                // Expected
            }
            com.destroy();
        }
        {
            //
            // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
            // facet names.
            //
            Dictionary<string, string> props = new Dictionary<string, string>
            {
                { "Ice.Admin.Endpoints", "tcp -h 127.0.0.1" },
                { "Ice.Admin.InstanceName", "Test" },
                { "Ice.Admin.Facets", "TestFacet, Process" }
            };
            var com = factory.createCommunicator(props);
            Ice.ObjectPrx obj = com.getAdmin();
            try
            {
                PropertiesAdminPrxHelper.checkedCast(obj, "Properties");
                test(false);
            }
            catch (FacetNotExistException)
            {
                // Expected
            }
            var tf = Test.TestFacetPrxHelper.checkedCast(obj, "TestFacet");
            tf.op();
            Ice.ProcessPrx proc = Ice.ProcessPrxHelper.checkedCast(obj, "Process");
            proc.shutdown();
            com.waitForShutdown();
            com.destroy();
        }
        output.WriteLine("ok");

        factory.shutdown();
    }

    private class RemoteLoggerI : Ice.RemoteLoggerDisp_
    {
        public override void init(string prefix, Ice.LogMessage[] messages, Ice.Current current)
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

        public override void log(Ice.LogMessage message, Ice.Current current)
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
