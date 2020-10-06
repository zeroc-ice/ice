// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using Test;

namespace ZeroC.Ice.Test.Binding
{
    public class AllTests
    {
        private static string GetAdapterNameWithAMI(ITestIntfPrx testIntf) =>
            testIntf.GetAdapterNameAsync().Result;

        private static void Shuffle(ref List<IRemoteObjectAdapterPrx> array)
        {
            for (int i = 0; i < array.Count - 1; ++i)
            {
                int r = _rand.Next(array.Count - i) + i;
                TestHelper.Assert(r >= i && r < array.Count);
                if (r != i)
                {
                    IRemoteObjectAdapterPrx tmp = array[i];
                    array[i] = array[r];
                    array[r] = tmp;
                }
            }
        }

        private static ITestIntfPrx CreateTestIntfPrx(List<IRemoteObjectAdapterPrx> adapters)
        {
            var endpoints = new List<Endpoint>();
            ITestIntfPrx? obj = null;
            IEnumerator<IRemoteObjectAdapterPrx> p = adapters.GetEnumerator();
            while (p.MoveNext())
            {
                obj = p.Current.GetTestIntf();
                endpoints.AddRange(obj!.Endpoints);
            }
            TestHelper.Assert(obj != null);
            return obj.Clone(endpoints: endpoints);
        }

        private static void Deactivate(IRemoteCommunicatorPrx communicator, List<IRemoteObjectAdapterPrx> adapters)
        {
            IEnumerator<IRemoteObjectAdapterPrx> p = adapters.GetEnumerator();
            while (p.MoveNext())
            {
                communicator.DeactivateObjectAdapter(p.Current);
            }
        }

        public static void Run(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator;
            bool ice1 = helper.Protocol == Protocol.Ice1;
            TestHelper.Assert(communicator != null);
            var com = IRemoteCommunicatorPrx.Parse(helper.GetTestProxy("communicator", 0), communicator);
            string testTransport = helper.Transport;

            var rand = new Random(unchecked((int)DateTime.Now.Ticks));
            System.IO.TextWriter output = helper.Output;

            output.Write("testing binding with single endpoint... ");
            output.Flush();
            {
                // Use "default" here to ensure that it still works
                IRemoteObjectAdapterPrx? adapter = com.CreateObjectAdapter("Adapter", "default");
                TestHelper.Assert(adapter != null);
                ITestIntfPrx? test1 = adapter.GetTestIntf();
                ITestIntfPrx? test2 = adapter.GetTestIntf();
                TestHelper.Assert(test1 != null && test2 != null);
                TestHelper.Assert(test1.GetConnection() == test2.GetConnection());

                test1.IcePing();
                test2.IcePing();

                com.DeactivateObjectAdapter(adapter);

                var test3 = test1.Clone(ITestIntfPrx.Factory);
                TestHelper.Assert(test3.GetConnection() == test1.GetConnection());
                TestHelper.Assert(test3.GetConnection() == test2.GetConnection());

                try
                {
                    test3.IcePing();
                    TestHelper.Assert(false);
                }
                catch (ConnectFailedException)
                {
                }
            }
            output.WriteLine("ok");

            output.Write("testing binding with multiple endpoints... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("Adapter11", testTransport)!,
                    com.CreateObjectAdapter("Adapter12", testTransport)!,
                    com.CreateObjectAdapter("Adapter13", testTransport)!
                };

                // Ensure that when a connection is opened it's reused for new proxies and that all endpoints are
                // eventually tried.
                var names = new List<string>
                {
                    "Adapter11",
                    "Adapter12",
                    "Adapter13"
                };

                while (names.Count > 0)
                {
                    var adpts = new List<IRemoteObjectAdapterPrx>(adapters);

                    ITestIntfPrx test1 = CreateTestIntfPrx(adpts);
                    Shuffle(ref adpts);
                    ITestIntfPrx test2 = CreateTestIntfPrx(adpts);
                    Shuffle(ref adpts);
                    ITestIntfPrx test3 = CreateTestIntfPrx(adpts);
                    test1.IcePing();
                    TestHelper.Assert(test1.GetConnection() == test2.GetConnection());
                    TestHelper.Assert(test2.GetConnection() == test3.GetConnection());

                    names.Remove(test1.GetAdapterName());
                    test1.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                // Ensure that the proxy correctly caches the connection(we always send the request over the same
                // connection.)
                {
                    foreach (IRemoteObjectAdapterPrx adpt in adapters)
                    {
                        adpt.GetTestIntf()!.IcePing();
                    }

                    ITestIntfPrx t = CreateTestIntfPrx(adapters);
                    string name = t.GetAdapterName();

                    for (int i = 0; i < 10 && t.GetAdapterName().Equals(name); i++)
                    {
                        TestHelper.Assert(t.GetAdapterName() == name);
                    }

                    foreach (IRemoteObjectAdapterPrx adpt in adapters)
                    {
                        adpt.GetTestIntf()!.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }
                }

                // Deactivate an adapter and ensure that we can still establish the connection to the remaining
                // adapters.
                com.DeactivateObjectAdapter(adapters[0]);
                names.Add("Adapter12");
                names.Add("Adapter13");
                while (names.Count > 0)
                {
                    var adpts = new List<IRemoteObjectAdapterPrx>(adapters);

                    ITestIntfPrx test1 = CreateTestIntfPrx(adpts);
                    Shuffle(ref adpts);
                    ITestIntfPrx test2 = CreateTestIntfPrx(adpts);
                    Shuffle(ref adpts);
                    ITestIntfPrx test3 = CreateTestIntfPrx(adpts);

                    TestHelper.Assert(test1.GetConnection() == test2.GetConnection());
                    TestHelper.Assert(test2.GetConnection() == test3.GetConnection());

                    names.Remove(test1.GetAdapterName());
                    test1.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                // Deactivate an adapter and ensure that we can still establish the connection to the remaining
                // adapter.
                com.DeactivateObjectAdapter(adapters[2]);
                ITestIntfPrx obj = CreateTestIntfPrx(adapters);
                TestHelper.Assert(obj.GetAdapterName().Equals("Adapter12"));

                Deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing binding with multiple random endpoints... ");
            output.Flush();
            {
                var adapters = new IRemoteObjectAdapterPrx[5]
                {
                    com.CreateObjectAdapter("AdapterRandom11", testTransport)!,
                    com.CreateObjectAdapter("AdapterRandom12", testTransport)!,
                    com.CreateObjectAdapter("AdapterRandom13", testTransport)!,
                    com.CreateObjectAdapter("AdapterRandom14", testTransport)!,
                    com.CreateObjectAdapter("AdapterRandom15", testTransport)!
                };

                int count = 20;
                int adapterCount = adapters.Length;
                while (--count > 0)
                {
                    ITestIntfPrx[] proxies;
                    if (count == 1)
                    {
                        com.DeactivateObjectAdapter(adapters[4]);
                        --adapterCount;
                    }
                    proxies = new ITestIntfPrx[10];

                    int i;
                    for (i = 0; i < proxies.Length; ++i)
                    {
                        var adpts = new IRemoteObjectAdapterPrx[rand.Next(adapters.Length)];
                        if (adpts.Length == 0)
                        {
                            adpts = new IRemoteObjectAdapterPrx[1];
                        }
                        for (int j = 0; j < adpts.Length; ++j)
                        {
                            adpts[j] = adapters[rand.Next(adapters.Length)];
                        }
                        proxies[i] = CreateTestIntfPrx(new List<IRemoteObjectAdapterPrx>(adpts));
                    }

                    for (i = 0; i < proxies.Length; i++)
                    {
                        proxies[i].GetAdapterNameAsync();
                    }
                    for (i = 0; i < proxies.Length; i++)
                    {
                        try
                        {
                            proxies[i].IcePing();
                        }
                        catch
                        {
                        }
                    }

                    var connections = new List<Connection>();
                    for (i = 0; i < proxies.Length; i++)
                    {
                        if (proxies[i].GetCachedConnection() is Connection connection)
                        {
                            if (!connections.Contains(connection))
                            {
                                connections.Add(connection);
                            }
                        }
                    }
                    TestHelper.Assert(connections.Count <= adapterCount);

                    foreach (IRemoteObjectAdapterPrx a in adapters)
                    {
                        try
                        {
                            a.GetTestIntf()!.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                        }
                        catch
                        {
                            // Expected if adapter is down.
                        }
                    }
                }
            }
            output.WriteLine("ok");

            output.Write("testing binding with multiple endpoints and AMI... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("AdapterAMI11", testTransport)!,
                    com.CreateObjectAdapter("AdapterAMI12", testTransport)!,
                    com.CreateObjectAdapter("AdapterAMI13", testTransport)!
                };

                // Ensure that when a connection is opened it's reused for new proxies and that all endpoints are
                // eventually tried.
                var names = new List<string>
                {
                    "AdapterAMI11",
                    "AdapterAMI12",
                    "AdapterAMI13"
                };

                while (names.Count > 0)
                {
                    var adpts = new List<IRemoteObjectAdapterPrx>(adapters);

                    ITestIntfPrx test1 = CreateTestIntfPrx(adpts);
                    Shuffle(ref adpts);
                    ITestIntfPrx test2 = CreateTestIntfPrx(adpts);
                    Shuffle(ref adpts);
                    ITestIntfPrx test3 = CreateTestIntfPrx(adpts);
                    test1.IcePing();
                    TestHelper.Assert(test1.GetConnection() == test2.GetConnection());
                    TestHelper.Assert(test2.GetConnection() == test3.GetConnection());

                    names.Remove(GetAdapterNameWithAMI(test1));
                    test1.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                // Ensure that the proxy correctly caches the connection (we always send the request over the
                // same connection.)
                {
                    foreach (IRemoteObjectAdapterPrx? adpt in adapters)
                    {
                        adpt.GetTestIntf()!.IcePing();
                    }

                    ITestIntfPrx t = CreateTestIntfPrx(adapters);
                    string name = GetAdapterNameWithAMI(t);

                    for (int i = 0; i < 10; i++)
                    {
                        TestHelper.Assert(GetAdapterNameWithAMI(t) == name);
                    }

                    foreach (IRemoteObjectAdapterPrx? adpt in adapters)
                    {
                        adpt.GetTestIntf()!.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }
                }

                // Deactivate an adapter and ensure that we can still establish the connection to the remaining
                // adapters.
                com.DeactivateObjectAdapter(adapters[0]);
                names.Add("AdapterAMI12");
                names.Add("AdapterAMI13");
                while (names.Count > 0)
                {
                    var adpts = new List<IRemoteObjectAdapterPrx>(adapters);

                    ITestIntfPrx test1 = CreateTestIntfPrx(adpts);
                    Shuffle(ref adpts);
                    ITestIntfPrx test2 = CreateTestIntfPrx(adpts);
                    Shuffle(ref adpts);
                    ITestIntfPrx test3 = CreateTestIntfPrx(adpts);

                    TestHelper.Assert(test1.GetConnection() == test2.GetConnection());
                    TestHelper.Assert(test2.GetConnection() == test3.GetConnection());

                    names.Remove(GetAdapterNameWithAMI(test1));
                    test1.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                // Deactivate an adapter and ensure that we can still establish the connection to the remaining
                // adapter.
                com.DeactivateObjectAdapter(adapters[2]);
                ITestIntfPrx obj = CreateTestIntfPrx(adapters);
                TestHelper.Assert(GetAdapterNameWithAMI(obj).Equals("AdapterAMI12"));

                Deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing random endpoint selection... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("Adapter21", testTransport)!,
                    com.CreateObjectAdapter("Adapter22", testTransport)!,
                    com.CreateObjectAdapter("Adapter23", testTransport)!
                };

                ITestIntfPrx obj = CreateTestIntfPrx(adapters);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Random);

                var names = new List<string>
                {
                    "Adapter21",
                    "Adapter22",
                    "Adapter23"
                };

                while (names.Count > 0)
                {
                    names.Remove(obj.GetAdapterName());
                    obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                obj = obj.Clone(endpointSelection: EndpointSelectionType.Random);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Random);

                names.Add("Adapter21");
                names.Add("Adapter22");
                names.Add("Adapter23");
                while (names.Count > 0)
                {
                    names.Remove(obj.GetAdapterName());
                    obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                Deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing ordered endpoint selection... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("Adapter31", testTransport)!,
                    com.CreateObjectAdapter("Adapter32", testTransport)!,
                    com.CreateObjectAdapter("Adapter33", testTransport)!
                };

                ITestIntfPrx obj = CreateTestIntfPrx(adapters);
                obj = obj.Clone(endpointSelection: EndpointSelectionType.Ordered);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Ordered);

                // Ensure that endpoints are tried in order by deactivating the adapters
                // one after the other.
                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(obj.GetAdapterName() == "Adapter31");
                }
                com.DeactivateObjectAdapter(adapters[0]);

                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(obj.GetAdapterName() == "Adapter32");
                }
                com.DeactivateObjectAdapter(adapters[1]);

                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(obj.GetAdapterName() == "Adapter33");
                }
                com.DeactivateObjectAdapter(adapters[2]);

                try
                {
                    obj.GetAdapterName();
                }
                catch (ConnectFailedException)
                {
                }

                IReadOnlyList<Endpoint> endpoints = obj.Endpoints;

                adapters.Clear();

                // TODO: ice1-only for now, because we send the client endpoints for use in OA configuration.
                if (ice1)
                {
                    // Now, re-activate the adapters with the same endpoints in the opposite order.
                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter36", endpoints[2].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter36");
                    }
                    obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);

                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter35", endpoints[1].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter35");
                    }
                    obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);

                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter34", endpoints[0].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter34");
                    }
                    Deactivate(com, adapters);
                }
            }
            output.WriteLine("ok");

            output.Write("testing per request binding with single endpoint... ");
            output.Flush();
            {
                IRemoteObjectAdapterPrx? adapter = com.CreateObjectAdapter("Adapter41", testTransport);
                TestHelper.Assert(adapter != null);
                ITestIntfPrx test1 = adapter.GetTestIntf()!.Clone(cacheConnection: false);
                ITestIntfPrx test2 = adapter.GetTestIntf()!.Clone(cacheConnection: false);
                TestHelper.Assert(!test1.IsConnectionCached);
                TestHelper.Assert(!test2.IsConnectionCached);
                TestHelper.Assert(test1.GetConnection() != null && test2.GetConnection() != null);
                TestHelper.Assert(test1.GetConnection() == test2.GetConnection());

                test1.IcePing();

                com.DeactivateObjectAdapter(adapter);

                var test3 = test1.Clone(ITestIntfPrx.Factory);
                try
                {
                    TestHelper.Assert(test3.GetConnection() == test1.GetConnection());
                    TestHelper.Assert(false);
                }
                catch (ConnectFailedException)
                {
                }
            }
            output.WriteLine("ok");

            output.Write("testing per request binding with multiple endpoints... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("Adapter51", testTransport)!,
                    com.CreateObjectAdapter("Adapter52", testTransport)!,
                    com.CreateObjectAdapter("Adapter53", testTransport)!
                };

                ITestIntfPrx obj = CreateTestIntfPrx(adapters).Clone(cacheConnection: false);
                TestHelper.Assert(!obj.IsConnectionCached);

                var names = new List<string>
                {
                    "Adapter51",
                    "Adapter52",
                    "Adapter53"
                };
                while (names.Count > 0)
                {
                    names.Remove(obj.GetAdapterName());
                }

                com.DeactivateObjectAdapter(adapters[0]);

                names.Add("Adapter52");
                names.Add("Adapter53");
                while (names.Count > 0)
                {
                    names.Remove(obj.GetAdapterName());
                }

                com.DeactivateObjectAdapter(adapters[2]);

                TestHelper.Assert(obj.GetAdapterName().Equals("Adapter52"));

                Deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing per request binding with multiple endpoints and AMI... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("AdapterAMI51", testTransport)!,
                    com.CreateObjectAdapter("AdapterAMI52", testTransport)!,
                    com.CreateObjectAdapter("AdapterAMI53", testTransport)!
                };

                ITestIntfPrx obj = CreateTestIntfPrx(adapters).Clone(cacheConnection: false);
                TestHelper.Assert(!obj.IsConnectionCached);

                var names = new List<string>
                {
                    "AdapterAMI51",
                    "AdapterAMI52",
                    "AdapterAMI53"
                };
                while (names.Count > 0)
                {
                    names.Remove(GetAdapterNameWithAMI(obj));
                }

                com.DeactivateObjectAdapter(adapters[0]);

                names.Add("AdapterAMI52");
                names.Add("AdapterAMI53");
                while (names.Count > 0)
                {
                    names.Remove(GetAdapterNameWithAMI(obj));
                }

                com.DeactivateObjectAdapter(adapters[2]);

                TestHelper.Assert(GetAdapterNameWithAMI(obj).Equals("AdapterAMI52"));

                Deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing per request binding and ordered endpoint selection... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("Adapter61", testTransport)!,
                    com.CreateObjectAdapter("Adapter62", testTransport)!,
                    com.CreateObjectAdapter("Adapter63", testTransport)!
                };

                ITestIntfPrx obj = CreateTestIntfPrx(adapters);
                obj = obj.Clone(endpointSelection: EndpointSelectionType.Ordered);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Ordered);
                obj = obj.Clone(cacheConnection: false);
                TestHelper.Assert(!obj.IsConnectionCached);

                // Ensure that endpoints are tried in order by deactivating the adapters one after the other.
                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(obj.GetAdapterName() == "Adapter61");
                }
                com.DeactivateObjectAdapter(adapters[0]);

                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(obj.GetAdapterName() == "Adapter62");
                }
                com.DeactivateObjectAdapter(adapters[1]);

                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(obj.GetAdapterName() == "Adapter63");
                }
                com.DeactivateObjectAdapter(adapters[2]);

                try
                {
                    obj.GetAdapterName();
                }
                catch (ConnectFailedException)
                {
                }

                IReadOnlyList<Endpoint> endpoints = obj.Endpoints;

                adapters.Clear();

                // TODO: ice1-only for now, because we send the client endpoints for use in OA configuration.
                if (helper.Protocol == Protocol.Ice1)
                {
                    // Now, re-activate the adapters with the same endpoints in the opposite order.
                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter66", endpoints[2].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter66");
                    }

                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter65", endpoints[1].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter65");
                    }

                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter64", endpoints[0].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter64");
                    }

                    Deactivate(com, adapters);
                }
            }
            output.WriteLine("ok");

            output.Write("testing per request binding and ordered endpoint selection and AMI... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("AdapterAMI61", testTransport)!,
                    com.CreateObjectAdapter("AdapterAMI62", testTransport)!,
                    com.CreateObjectAdapter("AdapterAMI63", testTransport)!
                };

                ITestIntfPrx? obj = CreateTestIntfPrx(adapters);
                obj = obj.Clone(endpointSelection: EndpointSelectionType.Ordered);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Ordered);
                obj = obj.Clone(cacheConnection: false);
                TestHelper.Assert(!obj.IsConnectionCached);

                // Ensure that endpoints are tried in order by deactivating the adapters one after the other.
                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(GetAdapterNameWithAMI(obj) == "AdapterAMI61");
                }
                com.DeactivateObjectAdapter(adapters[0]);

                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(GetAdapterNameWithAMI(obj) == "AdapterAMI62");
                }
                com.DeactivateObjectAdapter(adapters[1]);

                for (int i = 0; i < 3; i++)
                {
                    TestHelper.Assert(GetAdapterNameWithAMI(obj) == "AdapterAMI63");
                }
                com.DeactivateObjectAdapter(adapters[2]);

                try
                {
                    obj.GetAdapterName();
                }
                catch (ConnectFailedException)
                {
                }

                IReadOnlyList<Endpoint> endpoints = obj.Endpoints;

                adapters.Clear();

                // TODO: ice1-only for now, because we send the client endpoints for use in OA configuration.
                if (helper.Protocol == Protocol.Ice1)
                {
                    // Now, re-activate the adapters with the same endpoints in the opposite order.
                    adapters.Add(com.CreateObjectAdapterWithEndpoints("AdapterAMI66", endpoints[2].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(GetAdapterNameWithAMI(obj) == "AdapterAMI66");
                    }

                    adapters.Add(com.CreateObjectAdapterWithEndpoints("AdapterAMI65", endpoints[1].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(GetAdapterNameWithAMI(obj) == "AdapterAMI65");
                    }

                    adapters.Add(com.CreateObjectAdapterWithEndpoints("AdapterAMI64", endpoints[0].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(GetAdapterNameWithAMI(obj) == "AdapterAMI64");
                    }
                    Deactivate(com, adapters);
                }
            }
            output.WriteLine("ok");

            if (helper.Protocol == Protocol.Ice1)
            {
                output.Write("testing endpoint mode filtering... ");
                output.Flush();
                {
                    var adapters = new List<IRemoteObjectAdapterPrx>
                    {
                        com.CreateObjectAdapter("Adapter71", testTransport),
                        com.CreateObjectAdapter("Adapter72", "udp")
                    };

                    ITestIntfPrx obj = CreateTestIntfPrx(adapters);
                    TestHelper.Assert(obj.GetAdapterName().Equals("Adapter71"));

                    ITestIntfPrx testUDP = obj.Clone(invocationMode: InvocationMode.Datagram);
                    TestHelper.Assert(obj.GetConnection() != testUDP.GetConnection());
                    try
                    {
                        testUDP.GetAdapterName();
                        TestHelper.Assert(false);
                    }
                    catch (InvalidOperationException)
                    {
                        // expected
                    }
                }
                output.WriteLine("ok");
            }
            if (communicator.GetProperty("Ice.Plugin.IceSSL") != null)
            {
                output.Write("testing secure and non-secure endpoints... ");
                output.Flush();
                {
                    var adapters = new List<IRemoteObjectAdapterPrx>
                    {
                        com.CreateObjectAdapter("Adapter81", "ssl")!,
                        com.CreateObjectAdapter("Adapter82", "tcp")!
                    };

                    ITestIntfPrx obj = CreateTestIntfPrx(adapters);

                    for (int i = 0; i < 5; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName().Equals("Adapter82"));
                        obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }

                    ITestIntfPrx testNonSecure = obj.Clone(preferNonSecure: true);
                    // TODO: update when PreferNonSecure default is updated
                    ITestIntfPrx testSecure = obj.Clone(preferNonSecure: false);
                    TestHelper.Assert(obj.GetConnection() != testSecure.GetConnection());
                    TestHelper.Assert(obj.GetConnection() == testNonSecure.GetConnection());

                    com.DeactivateObjectAdapter(adapters[1]);

                    for (int i = 0; i < 5; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName().Equals("Adapter81"));
                        obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }

                    // TODO: ice1-only for now, because we send the client endpoints for use in OA configuration.
                    if (helper.Protocol == Protocol.Ice1)
                    {
                        com.CreateObjectAdapterWithEndpoints("Adapter83", obj.Endpoints[1].ToString()); // Recreate a tcp OA.

                        for (int i = 0; i < 5; i++)
                        {
                            TestHelper.Assert(obj.GetAdapterName().Equals("Adapter83"));
                            obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                        }
                    }

                    com.DeactivateObjectAdapter(adapters[0]);

                    try
                    {
                        testSecure.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (ConnectionRefusedException)
                    {
                        // expected
                    }
                    Deactivate(com, adapters);
                }
                output.WriteLine("ok");
            }

            {
                output.Write("testing ipv4 & ipv6 connections... ");
                output.Flush();

                // TODO: rework what we test here.
                // 1. The Ice. prefix has been missing for a while so this isn't testing much
                // 2. We're removing the Ice.IPv4, Ice.IPv4, and Ice.PreferIPv6Address properties
                var ipv4 = new Dictionary<string, string>()
                {
                    { "IPv4", "1" },
                    { "IPv6", "0" }
                    };

                var ipv6 = new Dictionary<string, string>()
                {
                    { "IPv4", "0" },
                    { "IPv6", "1" }
                };

                var bothPreferIPv4 = new Dictionary<string, string>()
                {
                    { "IPv4", "1" },
                    { "IPv6", "1" },
                    { "PreferIPv6Address", "0" }
                };

                var bothPreferIPv6 = new Dictionary<string, string>()
                {
                    { "IPv4", "1" },
                    { "IPv6", "1" },
                    { "PreferIPv6Address", "1" }
                };

                Dictionary<string, string>[] clientProps =
                {
                    ipv4, ipv6, bothPreferIPv4, bothPreferIPv6
                };

                Func<string, string> getEndpoint = host =>
                    TestHelper.GetTestEndpoint(
                        new Dictionary<string, string>(communicator.GetProperties())
                        {
                            ["Test.Host"] = host
                        },
                        2,
                        "tcp");

                 Func<string, string, string> getProxy = (identity, host) =>
                    TestHelper.GetTestProxy(
                        identity,
                        new Dictionary<string, string>(communicator.GetProperties())
                        {
                            ["Test.Host"] = host
                        },
                        2,
                        "tcp");

                var anyipv4 = new Dictionary<string, string>(ipv4)
                {
                    ["Adapter.Endpoints"] = getEndpoint("0.0.0.0"),
                    ["Adapter.PublishedEndpoints"] = getEndpoint("127.0.0.1")
                };

                var anyipv6 = new Dictionary<string, string>(ipv6)
                {
                    ["Adapter.Endpoints"] = getEndpoint("::0"),
                    ["Adapter.PublishedEndpoints"] = getEndpoint("::1")
                };

                var anyboth = new Dictionary<string, string>()
                {
                    { "IPv4", "1" },
                    { "IPv6", "1"},
                    { "Adapter.Endpoints", getEndpoint("::0")},
                    { "Adapter.PublishedEndpoints", getEndpoint("localhost") }
                };

                var localipv4 = new Dictionary<string, string>(ipv4)
                {
                    ["Adapter.Endpoints"] = getEndpoint("127.0.0.1")
                };

                var localipv6 = new Dictionary<string, string>(ipv6)
                {
                    ["Adapter.Endpoints"] = getEndpoint("::1")
                };

                Dictionary<string, string>[] serverProps =
                {
                    anyipv4,
                    anyipv6,
                    anyboth,
                    localipv4,
                    localipv6
                };

                bool ipv6NotSupported = false;
                foreach (Dictionary<string, string> p in serverProps)
                {
                    var serverCommunicator = new Communicator(p);
                    ObjectAdapter oa;
                    try
                    {
                        oa = serverCommunicator.CreateObjectAdapter("Adapter");
                        oa.Activate();
                    }
                    catch (DNSException)
                    {
                        serverCommunicator.Dispose();
                        continue; // IP version not supported.
                    }
                    catch (TransportException)
                    {
                        if (p == ipv6)
                        {
                            ipv6NotSupported = true;
                        }
                        serverCommunicator.Dispose();
                        continue; // IP version not supported.
                    }

                    IObjectPrx prx = oa.CreateProxy("dummy", IObjectPrx.Factory);
                    try
                    {
                        prx.IcePing();
                    }
                    catch (DNSException) // TODO: is this really an expected exception?
                    {
                        serverCommunicator.Dispose();
                        continue;
                    }
                    catch (ObjectNotExistException) // TODO: is this really an expected exception?
                    {
                        serverCommunicator.Dispose();
                        continue;
                    }

                    string strPrx = prx.ToString()!;
                    foreach (Dictionary<string, string> q in clientProps)
                    {
                        var clientCommunicator = new Communicator(q);
                        prx = IObjectPrx.Parse(strPrx, clientCommunicator);
                        try
                        {
                            prx.IcePing();
                            TestHelper.Assert(false);
                        }
                        catch (ObjectNotExistException)
                        {
                            // Expected, no object registered.
                        }
                        catch (DNSException)
                        {
                            // Expected if no IPv4 or IPv6 address is
                            // associated to localhost or if trying to connect
                            // to an any endpoint with the wrong IP version,
                            // e.g.: resolving an IPv4 address when only IPv6
                            // is enabled fails with a DNS exception.
                        }
                        catch (TransportException)
                        {
                            TestHelper.Assert((p == ipv4 && q == ipv6) || (p == ipv6 && q == ipv4) ||
                                (p == bothPreferIPv4 && q == ipv6) || (p == bothPreferIPv6 && q == ipv4) ||
                                (p == bothPreferIPv6 && q == ipv6 && ipv6NotSupported) ||
                                (p == anyipv4 && q == ipv6) || (p == anyipv6 && q == ipv4) ||
                                (p == localipv4 && q == ipv6) || (p == localipv6 && q == ipv4) ||
                                (p == ipv6 && q == bothPreferIPv4) || (p == ipv6 && q == bothPreferIPv6) ||
                                (p == bothPreferIPv6 && q == ipv6));
                        }
                        clientCommunicator.Dispose();
                    }
                    serverCommunicator.Dispose();
                }

                // Test IPv6 dual mode socket
                {
                    using Communicator serverCommunicator = new Communicator();
                    string endpoint = getEndpoint("::0");
                    ObjectAdapter oa = serverCommunicator.CreateObjectAdapterWithEndpoints(endpoint);
                    oa.Activate();

                    try
                    {
                        using ObjectAdapter ipv4OA =
                            serverCommunicator.CreateObjectAdapterWithEndpoints(getEndpoint("0.0.0.0"));
                        ipv4OA.Activate();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected. ::0 is a dual-mode socket so binding 0.0.0.0 will fail
                    }

                    try
                    {
                        using Communicator clientCommunicator = new Communicator();
                        var prx = IObjectPrx.Parse(getProxy("dummy", "127.0.0.1"), clientCommunicator);
                        prx.IcePing();
                    }
                    catch (ObjectNotExistException)
                    {
                        // Expected, no object registered.
                    }
                }

                // Test IPv6 only socket
                {
                    using Communicator serverCommunicator = new Communicator();
                    string endpoint = getEndpoint("::0") + (ice1 ? " --ipv6Only" : "?ipv6-only=true");
                    ObjectAdapter oa = serverCommunicator.CreateObjectAdapterWithEndpoints(endpoint);
                    oa.Activate();

                    // Returned endpoints must be IPv6
                    {
                        IObjectPrx prx = oa.CreateProxy("dummy", IObjectPrx.Factory);
                        TestHelper.Assert(prx.Endpoints.Count > 0);
                        TestHelper.Assert(prx.Endpoints.All(e => e.Host.Contains(":")));
                    }

                    // 0.0.0.0 can still be bound if ::0 is IPv6 only
                    {
                        string ipv4Endpoint = getEndpoint("0.0.0.0");
                        using ObjectAdapter ipv4OA = serverCommunicator.CreateObjectAdapterWithEndpoints(ipv4Endpoint);
                        ipv4OA.Activate();
                    }

                    try
                    {
                        using Communicator clientCommunicator = new Communicator();
                        var prx = IObjectPrx.Parse(getProxy("dummy", "127.0.0.1"), clientCommunicator);
                        prx.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (ConnectionRefusedException)
                    {
                        // Expected, server socket is IPv6 only.
                    }
                }

                // Listen on IPv4 loopback with IPv6 dual mode socket
                {
                    using Communicator serverCommunicator = new Communicator();
                    string endpoint = getEndpoint("::ffff:127.0.0.1");
                    ObjectAdapter oa = serverCommunicator.CreateObjectAdapterWithEndpoints(endpoint);
                    oa.Activate();

                    try
                    {
                        string ipv4Endpoint = getEndpoint("127.0.0.1");
                        using ObjectAdapter ipv4OA = serverCommunicator.CreateObjectAdapterWithEndpoints(ipv4Endpoint);
                        ipv4OA.Activate();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected. 127.0.0.1 is already in use
                    }

                    // Returned endpoint must be IPv6
                    {
                        IObjectPrx prx = oa.CreateProxy("dummy", IObjectPrx.Factory);
                        TestHelper.Assert(prx.Endpoints.Count == 1);
                        TestHelper.Assert(prx.Endpoints.All(e => e.Host.Contains(":")));
                    }

                    try
                    {
                        using Communicator clientCommunicator = new Communicator();
                        var prx = IObjectPrx.Parse(getProxy("dummy", "127.0.0.1"), clientCommunicator);
                        prx.IcePing();
                    }
                    catch (ObjectNotExistException)
                    {
                        // Expected, no object registered.
                    }
                }

                output.WriteLine("ok");
            }

            com.Shutdown();
        }

        private static readonly Random _rand = new Random(unchecked((int)DateTime.Now.Ticks));
    }
}
