//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Binding
{
    public class AllTests
    {
        private static string getAdapterNameWithAMI(ITestIntfPrx testIntf) =>
            testIntf.getAdapterNameAsync().Result;

        private static void shuffle(ref List<IRemoteObjectAdapterPrx> array)
        {
            for (int i = 0; i < array.Count - 1; ++i)
            {
                int r = _rand.Next(array.Count - i) + i;
                TestHelper.Assert(r >= i && r < array.Count);
                if (r != i)
                {
                    var tmp = array[i];
                    array[i] = array[r];
                    array[r] = tmp;
                }
            }
        }

        private static ITestIntfPrx createTestIntfPrx(List<IRemoteObjectAdapterPrx> adapters)
        {
            var endpoints = new List<Endpoint>();
            ITestIntfPrx? obj = null;
            IEnumerator<IRemoteObjectAdapterPrx> p = adapters.GetEnumerator();
            while (p.MoveNext())
            {
                obj = p.Current.getTestIntf();
                endpoints.AddRange(obj!.Endpoints);
            }
            TestHelper.Assert(obj != null);
            return obj.Clone(endpoints: endpoints);
        }

        private static void deactivate(IRemoteCommunicatorPrx communicator, List<IRemoteObjectAdapterPrx> adapters)
        {
            IEnumerator<IRemoteObjectAdapterPrx> p = adapters.GetEnumerator();
            while (p.MoveNext())
            {
                communicator.deactivateObjectAdapter(p.Current);
            }
        }

        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var com = IRemoteCommunicatorPrx.Parse($"communicator:{helper.GetTestEndpoint(0)}", communicator);

            var rand = new Random(unchecked((int)DateTime.Now.Ticks));
            System.IO.TextWriter output = helper.GetWriter();

            output.Write("testing binding with single endpoint... ");
            output.Flush();
            {
                IRemoteObjectAdapterPrx? adapter = com.createObjectAdapter("Adapter", "default");
                TestHelper.Assert(adapter != null);
                ITestIntfPrx? test1 = adapter.getTestIntf();
                ITestIntfPrx? test2 = adapter.getTestIntf();
                TestHelper.Assert(test1 != null && test2 != null);
                TestHelper.Assert(test1.GetConnection() == test2.GetConnection());

                test1.IcePing();
                test2.IcePing();

                com.deactivateObjectAdapter(adapter);

                var test3 = ITestIntfPrx.UncheckedCast(test1);
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
                    com.createObjectAdapter("Adapter11", "default")!,
                    com.createObjectAdapter("Adapter12", "default")!,
                    com.createObjectAdapter("Adapter13", "default")!
                };

                //
                // Ensure that when a connection is opened it's reused for new
                // proxies and that all endpoints are eventually tried.
                //
                List<string> names = new List<string>();
                names.Add("Adapter11");
                names.Add("Adapter12");
                names.Add("Adapter13");
                while (names.Count > 0)
                {
                    var adpts = new List<IRemoteObjectAdapterPrx>(adapters);

                    var test1 = createTestIntfPrx(adpts);
                    shuffle(ref adpts);
                    var test2 = createTestIntfPrx(adpts);
                    shuffle(ref adpts);
                    var test3 = createTestIntfPrx(adpts);
                    test1.IcePing();
                    TestHelper.Assert(test1.GetConnection() == test2.GetConnection());
                    TestHelper.Assert(test2.GetConnection() == test3.GetConnection());

                    names.Remove(test1.getAdapterName());
                    test1.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                //
                // Ensure that the proxy correctly caches the connection(we
                // always send the request over the same connection.)
                //
                {
                    foreach (IRemoteObjectAdapterPrx adpt in adapters)
                    {
                        adpt.getTestIntf()!.IcePing();
                    }

                    var t = createTestIntfPrx(adapters);
                    string name = t.getAdapterName();
                    int nRetry = 10;
                    int i;
                    for (i = 0; i < nRetry && t.getAdapterName().Equals(name); i++) ;
                    TestHelper.Assert(i == nRetry);

                    foreach (var adpt in adapters)
                    {
                        adpt.getTestIntf()!.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }
                }

                //
                // Deactivate an adapter and ensure that we can still
                // establish the connection to the remaining adapters.
                //
                com.deactivateObjectAdapter(adapters[0]);
                names.Add("Adapter12");
                names.Add("Adapter13");
                while (names.Count > 0)
                {
                    var adpts = new List<IRemoteObjectAdapterPrx>(adapters);

                    var test1 = createTestIntfPrx(adpts);
                    shuffle(ref adpts);
                    var test2 = createTestIntfPrx(adpts);
                    shuffle(ref adpts);
                    var test3 = createTestIntfPrx(adpts);

                    TestHelper.Assert(test1.GetConnection() == test2.GetConnection());
                    TestHelper.Assert(test2.GetConnection() == test3.GetConnection());

                    names.Remove(test1.getAdapterName());
                    test1.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                //
                // Deactivate an adapter and ensure that we can still
                // establish the connection to the remaining adapter.
                //
                com.deactivateObjectAdapter((IRemoteObjectAdapterPrx)adapters[2]);
                var obj = createTestIntfPrx(adapters);
                TestHelper.Assert(obj.getAdapterName().Equals("Adapter12"));

                deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing binding with multiple random endpoints... ");
            output.Flush();
            {
                var adapters = new IRemoteObjectAdapterPrx[5]
                {
                    com.createObjectAdapter("AdapterRandom11", "default")!,
                    com.createObjectAdapter("AdapterRandom12", "default")!,
                    com.createObjectAdapter("AdapterRandom13", "default")!,
                    com.createObjectAdapter("AdapterRandom14", "default")!,
                    com.createObjectAdapter("AdapterRandom15", "default")!
                };

                int count = 20;
                int adapterCount = adapters.Length;
                while (--count > 0)
                {
                    ITestIntfPrx[] proxies;
                    if (count == 1)
                    {
                        com.deactivateObjectAdapter(adapters[4]);
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
                        proxies[i] = createTestIntfPrx(new List<IRemoteObjectAdapterPrx>(adpts));
                    }

                    for (i = 0; i < proxies.Length; i++)
                    {
                        proxies[i].getAdapterNameAsync();
                    }
                    for (i = 0; i < proxies.Length; i++)
                    {
                        try
                        {
                            proxies[i].IcePing();
                        }
                        catch (Exception)
                        {
                        }
                    }

                    var connections = new List<Connection?>();
                    for (i = 0; i < proxies.Length; i++)
                    {
                        if (proxies[i].GetCachedConnection() != null)
                        {
                            if (!connections.Contains(proxies[i].GetCachedConnection()))
                            {
                                connections.Add(proxies[i].GetCachedConnection());
                            }
                        }
                    }
                    TestHelper.Assert(connections.Count <= adapterCount);

                    foreach (IRemoteObjectAdapterPrx a in adapters)
                    {
                        try
                        {
                            a.getTestIntf()!.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                        }
                        catch (Exception)
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
                    com.createObjectAdapter("AdapterAMI11", "default")!,
                    com.createObjectAdapter("AdapterAMI12", "default")!,
                    com.createObjectAdapter("AdapterAMI13", "default")!
                };

                //
                // Ensure that when a connection is opened it's reused for new
                // proxies and that all endpoints are eventually tried.
                //
                List<string> names = new List<string>();
                names.Add("AdapterAMI11");
                names.Add("AdapterAMI12");
                names.Add("AdapterAMI13");
                while (names.Count > 0)
                {
                    var adpts = new List<IRemoteObjectAdapterPrx>(adapters);

                    var test1 = createTestIntfPrx(adpts);
                    shuffle(ref adpts);
                    var test2 = createTestIntfPrx(adpts);
                    shuffle(ref adpts);
                    var test3 = createTestIntfPrx(adpts);
                    test1.IcePing();
                    TestHelper.Assert(test1.GetConnection() == test2.GetConnection());
                    TestHelper.Assert(test2.GetConnection() == test3.GetConnection());

                    names.Remove(getAdapterNameWithAMI(test1));
                    test1.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                //
                // Ensure that the proxy correctly caches the connection(we
                // always send the request over the same connection.)
                //
                {
                    foreach (var adpt in adapters)
                    {
                        adpt.getTestIntf()!.IcePing();
                    }

                    var t = createTestIntfPrx(adapters);
                    string name = getAdapterNameWithAMI(t);
                    int nRetry = 10;
                    int i;
                    for (i = 0; i < nRetry && getAdapterNameWithAMI(t).Equals(name); i++) ;
                    TestHelper.Assert(i == nRetry);

                    foreach (var adpt in adapters)
                    {
                        adpt.getTestIntf()!.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }
                }

                //
                // Deactivate an adapter and ensure that we can still
                // establish the connection to the remaining adapters.
                //
                com.deactivateObjectAdapter(adapters[0]);
                names.Add("AdapterAMI12");
                names.Add("AdapterAMI13");
                while (names.Count > 0)
                {
                    var adpts = new List<IRemoteObjectAdapterPrx>(adapters);

                    var test1 = createTestIntfPrx(adpts);
                    shuffle(ref adpts);
                    var test2 = createTestIntfPrx(adpts);
                    shuffle(ref adpts);
                    var test3 = createTestIntfPrx(adpts);

                    TestHelper.Assert(test1.GetConnection() == test2.GetConnection());
                    TestHelper.Assert(test2.GetConnection() == test3.GetConnection());

                    names.Remove(getAdapterNameWithAMI(test1));
                    test1.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                //
                // Deactivate an adapter and ensure that we can still
                // establish the connection to the remaining adapter.
                //
                com.deactivateObjectAdapter(adapters[2]);
                var obj = createTestIntfPrx(adapters);
                TestHelper.Assert(getAdapterNameWithAMI(obj).Equals("AdapterAMI12"));

                deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing random endpoint selection... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.createObjectAdapter("Adapter21", "default")!,
                    com.createObjectAdapter("Adapter22", "default")!,
                    com.createObjectAdapter("Adapter23", "default")!
                };

                var obj = createTestIntfPrx(adapters);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Random);

                var names = new List<string>
                {
                    "Adapter21",
                    "Adapter22",
                    "Adapter23"
                };
                while (names.Count > 0)
                {
                    names.Remove(obj.getAdapterName());
                    obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                obj = obj.Clone(endpointSelection: EndpointSelectionType.Random);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Random);

                names.Add("Adapter21");
                names.Add("Adapter22");
                names.Add("Adapter23");
                while (names.Count > 0)
                {
                    names.Remove(obj.getAdapterName());
                    obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                }

                deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing ordered endpoint selection... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.createObjectAdapter("Adapter31", "default")!,
                    com.createObjectAdapter("Adapter32", "default")!,
                    com.createObjectAdapter("Adapter33", "default")!
                };

                var obj = createTestIntfPrx(adapters);
                obj = obj.Clone(endpointSelection: EndpointSelectionType.Ordered);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Ordered);
                int nRetry = 3;
                int i;

                //
                // Ensure that endpoints are tried in order by deactivating the adapters
                // one after the other.
                //
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter31"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[0]);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter32"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[1]);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter33"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[2]);

                try
                {
                    obj.getAdapterName();
                }
                catch (ConnectFailedException)
                {
                }

                var endpoints = obj.Endpoints;

                adapters.Clear();

                //
                // Now, re-activate the adapters with the same endpoints in the opposite
                // order.
                //
                adapters.Add(com.createObjectAdapter("Adapter36", endpoints[2].ToString())!);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter36"); i++) ;
                TestHelper.Assert(i == nRetry);
                obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                adapters.Add(com.createObjectAdapter("Adapter35", endpoints[1].ToString())!);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter35"); i++) ;
                TestHelper.Assert(i == nRetry);
                obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                adapters.Add(com.createObjectAdapter("Adapter34", endpoints[0].ToString())!);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter34"); i++) ;
                TestHelper.Assert(i == nRetry);

                deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing per request binding with single endpoint... ");
            output.Flush();
            {
                IRemoteObjectAdapterPrx? adapter = com.createObjectAdapter("Adapter41", "default");
                TestHelper.Assert(adapter != null);
                ITestIntfPrx test1 = adapter.getTestIntf()!.Clone(cacheConnection: false);
                ITestIntfPrx test2 = adapter.getTestIntf()!.Clone(cacheConnection: false);
                TestHelper.Assert(!test1.IsConnectionCached);
                TestHelper.Assert(!test2.IsConnectionCached);
                TestHelper.Assert(test1.GetConnection() != null && test2.GetConnection() != null);
                TestHelper.Assert(test1.GetConnection() == test2.GetConnection());

                test1.IcePing();

                com.deactivateObjectAdapter(adapter);

                var test3 = ITestIntfPrx.UncheckedCast(test1);
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
                    com.createObjectAdapter("Adapter51", "default")!,
                    com.createObjectAdapter("Adapter52", "default")!,
                    com.createObjectAdapter("Adapter53", "default")!
                };

                ITestIntfPrx obj = createTestIntfPrx(adapters).Clone(cacheConnection: false);
                TestHelper.Assert(!obj.IsConnectionCached);

                var names = new List<string>
                {
                    "Adapter51",
                    "Adapter52",
                    "Adapter53"
                };
                while (names.Count > 0)
                {
                    names.Remove(obj.getAdapterName());
                }

                com.deactivateObjectAdapter(adapters[0]);

                names.Add("Adapter52");
                names.Add("Adapter53");
                while (names.Count > 0)
                {
                    names.Remove(obj.getAdapterName());
                }

                com.deactivateObjectAdapter(adapters[2]);

                TestHelper.Assert(obj.getAdapterName().Equals("Adapter52"));

                deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing per request binding with multiple endpoints and AMI... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.createObjectAdapter("AdapterAMI51", "default")!,
                    com.createObjectAdapter("AdapterAMI52", "default")!,
                    com.createObjectAdapter("AdapterAMI53", "default")!
                };

                ITestIntfPrx obj = createTestIntfPrx(adapters).Clone(cacheConnection: false);
                TestHelper.Assert(!obj.IsConnectionCached);

                var names = new List<string>
                {
                    "AdapterAMI51",
                    "AdapterAMI52",
                    "AdapterAMI53"
                };
                while (names.Count > 0)
                {
                    names.Remove(getAdapterNameWithAMI(obj));
                }

                com.deactivateObjectAdapter(adapters[0]);

                names.Add("AdapterAMI52");
                names.Add("AdapterAMI53");
                while (names.Count > 0)
                {
                    names.Remove(getAdapterNameWithAMI(obj));
                }

                com.deactivateObjectAdapter(adapters[2]);

                TestHelper.Assert(getAdapterNameWithAMI(obj).Equals("AdapterAMI52"));

                deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing per request binding and ordered endpoint selection... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.createObjectAdapter("Adapter61", "default")!,
                    com.createObjectAdapter("Adapter62", "default")!,
                    com.createObjectAdapter("Adapter63", "default")!
                };

                var obj = createTestIntfPrx(adapters);
                obj = obj.Clone(endpointSelection: EndpointSelectionType.Ordered);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Ordered);
                obj = obj.Clone(cacheConnection: false);
                TestHelper.Assert(!obj.IsConnectionCached);
                int nRetry = 3;
                int i;

                //
                // Ensure that endpoints are tried in order by deactivating the adapters
                // one after the other.
                //
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter61"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[0]);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter62"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[1]);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter63"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[2]);

                try
                {
                    obj.getAdapterName();
                }
                catch (ConnectFailedException)
                {
                }

                var endpoints = obj.Endpoints;

                adapters.Clear();

                //
                // Now, re-activate the adapters with the same endpoints in the opposite
                // order.
                //
                adapters.Add(com.createObjectAdapter("Adapter66", endpoints[2].ToString())!);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter66"); i++) ;
                TestHelper.Assert(i == nRetry);
                adapters.Add(com.createObjectAdapter("Adapter65", endpoints[1].ToString())!);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter65"); i++) ;
                TestHelper.Assert(i == nRetry);
                adapters.Add(com.createObjectAdapter("Adapter64", endpoints[0].ToString())!);
                for (i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter64"); i++) ;
                TestHelper.Assert(i == nRetry);

                deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing per request binding and ordered endpoint selection and AMI... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.createObjectAdapter("AdapterAMI61", "default")!,
                    com.createObjectAdapter("AdapterAMI62", "default")!,
                    com.createObjectAdapter("AdapterAMI63", "default")!
                };

                var obj = createTestIntfPrx(adapters);
                obj = obj.Clone(endpointSelection: EndpointSelectionType.Ordered);
                TestHelper.Assert(obj.EndpointSelection == EndpointSelectionType.Ordered);
                obj = obj.Clone(cacheConnection: false);
                TestHelper.Assert(!obj.IsConnectionCached);
                int nRetry = 3;
                int i;

                //
                // Ensure that endpoints are tried in order by deactiving the adapters
                // one after the other.
                //
                for (i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI61"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[0]);
                for (i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI62"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[1]);
                for (i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI63"); i++) ;
                TestHelper.Assert(i == nRetry);
                com.deactivateObjectAdapter(adapters[2]);

                try
                {
                    obj.getAdapterName();
                }
                catch (ConnectFailedException)
                {
                }

                var endpoints = obj.Endpoints;

                adapters.Clear();

                //
                // Now, re-activate the adapters with the same endpoints in the opposite
                // order.
                //
                adapters.Add(com.createObjectAdapter("AdapterAMI66", endpoints[2].ToString())!);
                for (i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI66"); i++) ;
                TestHelper.Assert(i == nRetry);
                adapters.Add(com.createObjectAdapter("AdapterAMI65", endpoints[1].ToString())!);
                for (i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI65"); i++) ;
                TestHelper.Assert(i == nRetry);
                adapters.Add(com.createObjectAdapter("AdapterAMI64", endpoints[0].ToString())!);
                for (i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI64"); i++) ;
                TestHelper.Assert(i == nRetry);

                deactivate(com, adapters);
            }
            output.WriteLine("ok");

            output.Write("testing endpoint mode filtering... ");
            output.Flush();
            {
                var adapters = new List<IRemoteObjectAdapterPrx>
                {
                    com.createObjectAdapter("Adapter71", "default")!,
                    com.createObjectAdapter("Adapter72", "udp")!
                };

                var obj = createTestIntfPrx(adapters);
                TestHelper.Assert(obj.getAdapterName().Equals("Adapter71"));

                var testUDP = obj.Clone(invocationMode: InvocationMode.Datagram);
                TestHelper.Assert(obj.GetConnection() != testUDP.GetConnection());
                try
                {
                    testUDP.getAdapterName();
                    TestHelper.Assert(false);
                }
                catch (System.InvalidOperationException)
                {
                    // expected
                }
            }
            output.WriteLine("ok");
            if (communicator.GetProperty("Ice.Plugin.IceSSL") != null)
            {
                output.Write("testing secure and non-secure endpoints... ");
                output.Flush();
                {
                    var adapters = new List<IRemoteObjectAdapterPrx>
                    {
                        com.createObjectAdapter("Adapter81", "ssl")!,
                        com.createObjectAdapter("Adapter82", "tcp")!
                    };

                    var obj = createTestIntfPrx(adapters);
                    int i;
                    for (i = 0; i < 5; i++)
                    {
                        TestHelper.Assert(obj.getAdapterName().Equals("Adapter82"));
                        obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }

                    var testNonSecure = obj.Clone(preferNonSecure: true);
                    // TODO: update when PreferNonSecure default is updated
                    var testSecure = obj.Clone(preferNonSecure: false);
                    TestHelper.Assert(obj.GetConnection() != testSecure.GetConnection());
                    TestHelper.Assert(obj.GetConnection() == testNonSecure.GetConnection());

                    com.deactivateObjectAdapter(adapters[1]);

                    for (i = 0; i < 5; i++)
                    {
                        TestHelper.Assert(obj.getAdapterName().Equals("Adapter81"));
                        obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }

                    com.createObjectAdapter("Adapter83", (obj.Endpoints[1]).ToString()); // Recreate a tcp OA.

                    for (i = 0; i < 5; i++)
                    {
                        TestHelper.Assert(obj.getAdapterName().Equals("Adapter83"));
                        obj.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
                    }

                    com.deactivateObjectAdapter(adapters[0]);

                    try
                    {
                        testSecure.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (ConnectionRefusedException)
                    {
                        // expected
                    }
                    deactivate(com, adapters);
                }
                output.WriteLine("ok");
            }

            {
                output.Write("testing ipv4 & ipv6 connections... ");
                output.Flush();

                var ipv4 = new Dictionary<string, string>()
                {
                    { "IPv4", "1" },
                    { "IPv6", "0" },
                    { "Adapter.Endpoints", "tcp -h localhost" }
                    };

                var ipv6 = new Dictionary<string, string>()
                {
                    { "IPv4", "0" },
                    { "IPv6", "1" },
                    { "Adapter.Endpoints", "tcp -h localhost" }
                };

                var bothPreferIPv4 = new Dictionary<string, string>()
                {
                    { "IPv4", "1" },
                    { "IPv6", "1" },
                    { "PreferIPv6Address", "0" },
                    { "Adapter.Endpoints", "tcp -h localhost" }
                };

                var bothPreferIPv6 = new Dictionary<string, string>()
                {
                    { "IPv4", "1" },
                    { "IPv6", "1" },
                    { "PreferIPv6Address", "1" },
                    { "Adapter.Endpoints", "tcp -h localhost" }
                };

                Dictionary<string, string>[] clientProps =
                {
                    ipv4, ipv6, bothPreferIPv4, bothPreferIPv6
                };

                string endpoint = "tcp -p " + helper.GetTestPort(2).ToString();

                var anyipv4 = new Dictionary<string, string>(ipv4);
                anyipv4["Adapter.Endpoints"] = endpoint;
                anyipv4["Adapter.PublishedEndpoints"] = $"{endpoint} -h 127.0.0.1";

                var anyipv6 = new Dictionary<string, string>(ipv6);
                anyipv6["Adapter.Endpoints"] = endpoint;
                anyipv6["Adapter.PublishedEndpoints"] = $"{endpoint} -h \".1\"";

                var anyboth = new Dictionary<string, string>()
                {
                    { "IPv4", "1" },
                    { "IPv6", "1"},
                    { "Adapter.Endpoints", endpoint },
                    { "Adapter.PublishedEndpoints", $"{endpoint} -h \"::1\":{endpoint} -h 127.0.0.1" }
                };

                var localipv4 = new Dictionary<string, string>(ipv4);
                localipv4["Adapter.Endpoints"] = "tcp -h 127.0.0.1";

                var localipv6 = new Dictionary<string, string>(ipv6);
                localipv6["Adapter.Endpoints"] = "tcp -h \"::1\"";

                Dictionary<string, string>[] serverProps =
                {
                    anyipv4,
                    anyipv6,
                    anyboth,
                    localipv4,
                    localipv6
                };

                bool ipv6NotSupported = false;
                foreach (var p in serverProps)
                {
                    Communicator serverCommunicator = new Communicator(p);
                    ObjectAdapter oa;
                    try
                    {
                        oa = serverCommunicator.CreateObjectAdapter("Adapter");
                        oa.Activate();
                    }
                    catch (DNSException)
                    {
                        serverCommunicator.Destroy();
                        continue; // IP version not supported.
                    }
                    catch (TransportException)
                    {
                        if (p == ipv6)
                        {
                            ipv6NotSupported = true;
                        }
                        serverCommunicator.Destroy();
                        continue; // IP version not supported.
                    }

                    var prx = oa.CreateProxy("dummy", IObjectPrx.Factory);
                    try
                    {
                        prx.Clone(collocationOptimized: false).IcePing();
                    }
                    catch (DNSException) // TODO: is this really an expected exception?
                    {
                        serverCommunicator.Destroy();
                        continue;
                    }
                    catch (ObjectNotExistException) // TODO: is this really an expected exception?
                    {
                        serverCommunicator.Destroy();
                        continue;
                    }

                    string strPrx = prx.ToString()!;
                    foreach (var q in clientProps)
                    {
                        Communicator clientCommunicator = new Communicator(q);
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
                        clientCommunicator.Destroy();
                    }
                    serverCommunicator.Destroy();
                }

                output.WriteLine("ok");
            }

            com.shutdown();
        }

        private static Random _rand = new Random(unchecked((int)DateTime.Now.Ticks));
    }
}
