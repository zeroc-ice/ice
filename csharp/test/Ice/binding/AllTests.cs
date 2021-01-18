// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Binding
{
    public static class AllTests
    {
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

        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;
            bool ice1 = helper.Protocol == Protocol.Ice1;

            var com = IRemoteCommunicatorPrx.Parse(helper.GetTestProxy("communicator", 0), communicator);
            string testTransport = helper.Transport;

            var rand = new Random(unchecked((int)DateTime.Now.Ticks));
            System.IO.TextWriter output = helper.Output;

            output.Write("testing binding with single endpoint... ");
            output.Flush();
            {
                // Use "default" with ice1 + tcp here to ensure that it still works
                IRemoteObjectAdapterPrx? adapter = com.CreateObjectAdapter("Adapter",
                    (ice1 && testTransport == "tcp") ? "default" : testTransport);
                TestHelper.Assert(adapter != null);
                ITestIntfPrx? test1 = adapter.GetTestIntf();
                ITestIntfPrx? test2 = adapter.GetTestIntf();
                TestHelper.Assert(test1 != null && test2 != null);
                TestHelper.Assert(await test1.GetConnectionAsync() == await test2.GetConnectionAsync());

                test1.IcePing();
                test2.IcePing();

                com.DeactivateObjectAdapter(adapter);

                var test3 = test1.Clone(ITestIntfPrx.Factory);
                TestHelper.Assert(test3.GetCachedConnection() == test1.GetCachedConnection());
                TestHelper.Assert(test3.GetCachedConnection() == test2.GetCachedConnection());

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
                    com.CreateObjectAdapter("Adapter31", testTransport)!,
                    com.CreateObjectAdapter("Adapter32", testTransport)!,
                    com.CreateObjectAdapter("Adapter33", testTransport)!
                };

                ITestIntfPrx obj = CreateTestIntfPrx(adapters);

                // Ensure that endpoints are tried in order by deactivating the adapters one after the other.
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
                adapters.Clear();
            }
            output.WriteLine("ok");

            output.Write("testing per request binding with single endpoint... ");
            output.Flush();
            {
                IRemoteObjectAdapterPrx? adapter = com.CreateObjectAdapter("Adapter41", testTransport);
                TestHelper.Assert(adapter != null);
                ITestIntfPrx test1 = adapter.GetTestIntf()!.Clone(cacheConnection: false,
                                                                  preferExistingConnection: false);
                ITestIntfPrx test2 = adapter.GetTestIntf()!.Clone(cacheConnection: false,
                                                                  preferExistingConnection: false);
                TestHelper.Assert(!test1.CacheConnection && !test1.PreferExistingConnection);
                TestHelper.Assert(!test2.CacheConnection && !test2.PreferExistingConnection);
                TestHelper.Assert(await test1.GetConnectionAsync() == await test2.GetConnectionAsync());

                test1.IcePing();

                com.DeactivateObjectAdapter(adapter);

                var test3 = test1.Clone(ITestIntfPrx.Factory);
                try
                {
                    TestHelper.Assert(await test3.GetConnectionAsync() == await test1.GetConnectionAsync());
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
                    com.CreateObjectAdapter("Adapter61", testTransport)!,
                    com.CreateObjectAdapter("Adapter62", testTransport)!,
                    com.CreateObjectAdapter("Adapter63", testTransport)!
                };

                ITestIntfPrx obj = CreateTestIntfPrx(adapters);
                obj = obj.Clone(cacheConnection: false, preferExistingConnection: false);
                TestHelper.Assert(!obj.CacheConnection && !obj.PreferExistingConnection);

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
                    // Wait 5 seconds to let recent endpoint failures expire
                    Thread.Sleep(5000);
                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter66", endpoints[2].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter66");
                    }

                    // Wait 5 seconds to let recent endpoint failures expire
                    Thread.Sleep(5000);
                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter65", endpoints[1].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter65");
                    }

                    // Wait 5 seconds to let recent endpoint failures expire
                    Thread.Sleep(5000);
                    adapters.Add(com.CreateObjectAdapterWithEndpoints("Adapter64", endpoints[0].ToString()));
                    for (int i = 0; i < 3; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName() == "Adapter64");
                    }

                    Deactivate(com, adapters);
                }
            }
            output.WriteLine("ok");

            output.Write("testing connection reuse with multiple endpoints... ");
            output.Flush();
            {
                var adapters1 = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("Adapter81", testTransport)!,
                    com.CreateObjectAdapter("Adapter82", testTransport)!,
                    com.CreateObjectAdapter("Adapter83", testTransport)!
                };

                var adapters2 = new List<IRemoteObjectAdapterPrx>
                {
                    adapters1[0],
                    com.CreateObjectAdapter("Adapter84", testTransport)!,
                    com.CreateObjectAdapter("Adapter85", testTransport)!
                };

                ITestIntfPrx obj1 = CreateTestIntfPrx(adapters1);
                ITestIntfPrx obj2 = CreateTestIntfPrx(adapters2);

                com.DeactivateObjectAdapter(adapters1[0]);

                Task<string> t1 = obj1.GetAdapterNameAsync();
                Task<string> t2 = obj2.GetAdapterNameAsync();
                TestHelper.Assert(t1.Result == "Adapter82");
                TestHelper.Assert(t2.Result == "Adapter84");

                Deactivate(com, adapters1);
                Deactivate(com, adapters2);
            }

            {
                var adapters1 = new List<IRemoteObjectAdapterPrx>
                {
                    com.CreateObjectAdapter("Adapter91", testTransport)!,
                    com.CreateObjectAdapter("Adapter92", testTransport)!,
                    com.CreateObjectAdapter("Adapter93", testTransport)!
                };

                var adapters2 = new List<IRemoteObjectAdapterPrx>
                {
                    adapters1[0],
                    com.CreateObjectAdapter("Adapter94", testTransport)!,
                    com.CreateObjectAdapter("Adapter95", testTransport)!
                };

                ITestIntfPrx obj1 = CreateTestIntfPrx(adapters1);
                ITestIntfPrx obj2 = CreateTestIntfPrx(adapters2);

                Task<string> t1 = obj1.GetAdapterNameAsync();
                Task<string> t2 = obj2.GetAdapterNameAsync();
                TestHelper.Assert(t1.Result == "Adapter91");
                TestHelper.Assert(t2.Result == "Adapter91");

                Deactivate(com, adapters1);
                Deactivate(com, adapters2);
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

                    // test that datagram proxies fail if PreferNonSecure is false
                    ITestIntfPrx testUDP = obj.Clone(invocationMode: InvocationMode.Datagram, preferNonSecure: NonSecure.Never);
                    try
                    {
                        await testUDP.GetConnectionAsync();
                        TestHelper.Assert(false);
                    }
                    catch (NoEndpointException)
                    {
                        // expected
                    }

                    testUDP = obj.Clone(invocationMode: InvocationMode.Datagram, preferNonSecure: NonSecure.Always);
                    TestHelper.Assert(await obj.GetConnectionAsync() != await testUDP.GetConnectionAsync());
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
                        _ = (await obj.GetConnectionAsync()).GoAwayAsync();
                    }

                    ITestIntfPrx testNonSecure = obj.Clone(preferNonSecure: NonSecure.Always);
                    // TODO: update when PreferNonSecure default is updated
                    ITestIntfPrx testSecure = obj.Clone(preferNonSecure: NonSecure.Never);
                    TestHelper.Assert(await obj.GetConnectionAsync() != await testSecure.GetConnectionAsync());
                    TestHelper.Assert(await obj.GetConnectionAsync() == await testNonSecure.GetConnectionAsync());

                    com.DeactivateObjectAdapter(adapters[1]);

                    for (int i = 0; i < 5; i++)
                    {
                        TestHelper.Assert(obj.GetAdapterName().Equals("Adapter81"));
                        _ = (await obj.GetConnectionAsync()).GoAwayAsync();
                    }

                    // TODO: ice1-only for now, because we send the client endpoints for use in OA configuration.
                    if (helper.Protocol == Protocol.Ice1)
                    {
                        com.CreateObjectAdapterWithEndpoints("Adapter83", obj.Endpoints[1].ToString()); // Recreate a tcp OA.

                        for (int i = 0; i < 5; i++)
                        {
                            TestHelper.Assert(obj.GetAdapterName().Equals("Adapter83"));
                            _ = (await obj.GetConnectionAsync()).GoAwayAsync();
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

                var anyipv4 = new Dictionary<string, string>()
                {
                    ["Adapter.Endpoints"] = getEndpoint("0.0.0.0"),
                    ["Adapter.PublishedEndpoints"] = getEndpoint("127.0.0.1")
                };

                var anyipv6 = new Dictionary<string, string>()
                {
                    ["Adapter.Endpoints"] = getEndpoint("::0"),
                    ["Adapter.PublishedEndpoints"] = getEndpoint("::1")
                };

                var anyipv46 = new Dictionary<string, string>()
                {
                    ["Adapter.Endpoints"] = getEndpoint("::0"),
                    ["Adapter.PublishedEndpoints"] = getEndpoint("127.0.0.1")
                };

                var anylocalhost = new Dictionary<string, string>()
                {
                    { "Adapter.Endpoints", getEndpoint("::0")},
                    { "Adapter.PublishedEndpoints", getEndpoint("localhost") }
                };

                var localipv4 = new Dictionary<string, string>()
                {
                    ["Adapter.Endpoints"] = getEndpoint("127.0.0.1"),
                    ["Adapter.ServerName"] = "127.0.0.1"
                };

                var localipv6 = new Dictionary<string, string>()
                {
                    ["Adapter.Endpoints"] = getEndpoint("::1"),
                    ["Adapter.ServerName"] = "::1"
                };

                var localhost = new Dictionary<string, string>()
                {
                    ["Adapter.Endpoints"] = getEndpoint("localhost"),
                    ["Adapter.ServerName"] = "localhost"
                };

                var serverProps = new Dictionary<string, string>[]
                {
                    anyipv4,
                    anyipv6,
                    anyipv46,
                    anylocalhost,
                    localipv4,
                    localipv6,
                    localhost
                };

                foreach (Dictionary<string, string> p in serverProps)
                {
                    await using var serverCommunicator = new Communicator(p);
                    ObjectAdapter oa = serverCommunicator.CreateObjectAdapter("Adapter");
                    await oa.ActivateAsync();

                    IObjectPrx prx = oa.CreateProxy("dummy", IObjectPrx.Factory);
                    try
                    {
                        await using var clientCommunicator = new Communicator();
                        prx = IObjectPrx.Parse(prx.ToString()!, clientCommunicator);
                        prx.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (ObjectNotExistException)
                    {
                        // Expected. OA is reachable but there's no "dummy" object
                    }
                }

                // Test IPv6 dual mode socket
                {
                    await using var serverCommunicator = new Communicator();
                    string endpoint = getEndpoint("::0");
                    ObjectAdapter oa = serverCommunicator.CreateObjectAdapterWithEndpoints(endpoint);
                    await oa.ActivateAsync();

                    try
                    {
                        await using ObjectAdapter ipv4OA =
                            serverCommunicator.CreateObjectAdapterWithEndpoints(getEndpoint("0.0.0.0"));
                        await ipv4OA.ActivateAsync();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected. ::0 is a dual-mode socket so binding 0.0.0.0 will fail
                    }

                    try
                    {
                        await using var clientCommunicator = new Communicator();
                        var prx = IObjectPrx.Parse(getProxy("dummy", "127.0.0.1"), clientCommunicator);
                        prx.IcePing();
                    }
                    catch (ObjectNotExistException)
                    {
                        // Expected, no object registered.
                    }
                }

                // Test IPv6 only endpoints
                {
                    await using var serverCommunicator = new Communicator();
                    string endpoint = getEndpoint("::0") + (ice1 ? " --ipv6Only" : "?ipv6-only=true");
                    ObjectAdapter oa = serverCommunicator.CreateObjectAdapterWithEndpoints(endpoint);
                    await oa.ActivateAsync();

                    // 0.0.0.0 can still be bound if ::0 is IPv6 only
                    {
                        string ipv4Endpoint = getEndpoint("0.0.0.0");
                        await using ObjectAdapter ipv4OA =
                            serverCommunicator.CreateObjectAdapterWithEndpoints(ipv4Endpoint);
                        await ipv4OA.ActivateAsync();
                    }

                    try
                    {
                        await using var clientCommunicator = new Communicator();
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
                    await using var serverCommunicator = new Communicator();
                    string endpoint = getEndpoint("::ffff:127.0.0.1");
                    ObjectAdapter oa = serverCommunicator.CreateObjectAdapterWithEndpoints(endpoint);
                    await oa.ActivateAsync();

                    try
                    {
                        string ipv4Endpoint = getEndpoint("127.0.0.1");
                        await using ObjectAdapter ipv4OA =
                            serverCommunicator.CreateObjectAdapterWithEndpoints(ipv4Endpoint);
                        await ipv4OA.ActivateAsync();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected. 127.0.0.1 is already in use
                    }

                    try
                    {
                        await using var clientCommunicator = new Communicator();
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
    }
}
