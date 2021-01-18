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
                TestHelper.Assert(facetMap.Count == 6);
                TestHelper.Assert(facetMap.ContainsKey("Properties"));
                TestHelper.Assert(facetMap.ContainsKey("Process"));
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
    }
}
