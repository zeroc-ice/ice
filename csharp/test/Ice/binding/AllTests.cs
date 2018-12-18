// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    namespace binding
    {
        public class AllTests : global::Test.AllTests
        {
            private static string getAdapterNameWithAMI(Test.TestIntfPrx testIntf)
            {
                object m = new object();
                string result = null;
                testIntf.begin_getAdapterName().whenCompleted(
                   (string name) =>
                    {
                        lock(m)
                        {
                            result = name;
                            System.Threading.Monitor.Pulse(m);
                        }
                    },
                   (Ice.Exception ex) =>
                    {
                        test(false);
                    });
                lock(m)
                {
                    while(result == null)
                    {
                        System.Threading.Monitor.Wait(m);
                    }
                    return result;
                }
            }

            private static void shuffle(ref List<Test.RemoteObjectAdapterPrx> array)
            {
                for(int i = 0; i < array.Count - 1; ++i)
                {
                    int r = _rand.Next(array.Count - i) + i;
                    Debug.Assert(r >= i && r < array.Count);
                    if(r != i)
                    {
                        var tmp = array[i];
                        array[i] = array[r];
                        array[r] = tmp;
                    }
                }
            }

            private static Test.TestIntfPrx createTestIntfPrx(List<Test.RemoteObjectAdapterPrx> adapters)
            {
                List<Ice.Endpoint> endpoints = new List<Ice.Endpoint>();
                Test.TestIntfPrx obj = null;
                IEnumerator<Test.RemoteObjectAdapterPrx> p = adapters.GetEnumerator();
                while(p.MoveNext())
                {
                    obj = p.Current.getTestIntf();
                    foreach(Ice.Endpoint e in obj.ice_getEndpoints())
                    {
                        endpoints.Add(e);
                    }
                }
                return Test.TestIntfPrxHelper.uncheckedCast(obj.ice_endpoints(endpoints.ToArray()));
            }

            private static void deactivate(Test.RemoteCommunicatorPrx communicator, List<Test.RemoteObjectAdapterPrx> adapters)
            {
                IEnumerator<Test.RemoteObjectAdapterPrx> p = adapters.GetEnumerator();
                while(p.MoveNext())
                {
                    communicator.deactivateObjectAdapter(p.Current);
                }
            }

            private class StringComparator : IComparer<String>
            {
                public int Compare(String l, String r)
                {
                    return l.CompareTo(r);
                }
            };

            public static void allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                string @ref = "communicator:" + helper.getTestEndpoint(0);
                Test.RemoteCommunicatorPrx com = Test.RemoteCommunicatorPrxHelper.uncheckedCast(communicator.stringToProxy(@ref));

                var rand = new Random(unchecked((int)DateTime.Now.Ticks));
                var output = helper.getWriter();

                output.Write("testing binding with single endpoint... ");
                output.Flush();
                {
                    Test.RemoteObjectAdapterPrx adapter = com.createObjectAdapter("Adapter", "default");

                    var test1 = adapter.getTestIntf();
                    var test2 = adapter.getTestIntf();
                    test(test1.ice_getConnection() == test2.ice_getConnection());

                    test1.ice_ping();
                    test2.ice_ping();

                    com.deactivateObjectAdapter(adapter);

                    var test3 = Test.TestIntfPrxHelper.uncheckedCast(test1);
                    test(test3.ice_getConnection() == test1.ice_getConnection());
                    test(test3.ice_getConnection() == test2.ice_getConnection());

                    try
                    {
                        test3.ice_ping();
                        test(false);
                    }
                    catch(Ice.ConnectFailedException)
                    {
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                    }
                }
                output.WriteLine("ok");

                output.Write("testing binding with multiple endpoints... ");
                output.Flush();
                {
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("Adapter11", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter12", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter13", "default"));

                    //
                    // Ensure that when a connection is opened it's reused for new
                    // proxies and that all endpoints are eventually tried.
                    //
                    List<string> names = new List<string>();
                    names.Add("Adapter11");
                    names.Add("Adapter12");
                    names.Add("Adapter13");
                    while(names.Count > 0)
                    {
                        var adpts = new List<Test.RemoteObjectAdapterPrx>(adapters);

                        var test1 = createTestIntfPrx(adpts);
                        shuffle(ref adpts);
                        var test2 = createTestIntfPrx(adpts);
                        shuffle(ref adpts);
                        var test3 = createTestIntfPrx(adpts);
                        test1.ice_ping();
                        test(test1.ice_getConnection() == test2.ice_getConnection());
                        test(test2.ice_getConnection() == test3.ice_getConnection());

                        names.Remove(test1.getAdapterName());
                        test1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    }

                    //
                    // Ensure that the proxy correctly caches the connection(we
                    // always send the request over the same connection.)
                    //
                    {
                        foreach(var adpt in adapters)
                        {
                            adpt.getTestIntf().ice_ping();
                        }

                        var t = createTestIntfPrx(adapters);
                        string name = t.getAdapterName();
                        int nRetry = 10;
                        int i;
                        for(i = 0; i < nRetry && t.getAdapterName().Equals(name); i++) ;
                        test(i == nRetry);

                        foreach(var adpt in adapters)
                        {
                            adpt.getTestIntf().ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                        }
                    }

                    //
                    // Deactivate an adapter and ensure that we can still
                    // establish the connection to the remaining adapters.
                    //
                    com.deactivateObjectAdapter((Test.RemoteObjectAdapterPrx)adapters[0]);
                    names.Add("Adapter12");
                    names.Add("Adapter13");
                    while(names.Count > 0)
                    {
                        var adpts = new List<Test.RemoteObjectAdapterPrx>(adapters);

                        var test1 = createTestIntfPrx(adpts);
                        shuffle(ref adpts);
                        var test2 = createTestIntfPrx(adpts);
                        shuffle(ref adpts);
                        var test3 = createTestIntfPrx(adpts);

                        test(test1.ice_getConnection() == test2.ice_getConnection());
                        test(test2.ice_getConnection() == test3.ice_getConnection());

                        names.Remove(test1.getAdapterName());
                        test1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    }

                    //
                    // Deactivate an adapter and ensure that we can still
                    // establish the connection to the remaining adapter.
                    //
                    com.deactivateObjectAdapter((Test.RemoteObjectAdapterPrx)adapters[2]);
                    var obj = createTestIntfPrx(adapters);
                    test(obj.getAdapterName().Equals("Adapter12"));

                    deactivate(com, adapters);
                }
                output.WriteLine("ok");

                output.Write("testing binding with multiple random endpoints... ");
                output.Flush();
                {
                    var adapters = new Test.RemoteObjectAdapterPrx[5];
                    adapters[0] = com.createObjectAdapter("AdapterRandom11", "default");
                    adapters[1] = com.createObjectAdapter("AdapterRandom12", "default");
                    adapters[2] = com.createObjectAdapter("AdapterRandom13", "default");
                    adapters[3] = com.createObjectAdapter("AdapterRandom14", "default");
                    adapters[4] = com.createObjectAdapter("AdapterRandom15", "default");

                    int count = 20;
                    int adapterCount = adapters.Length;
                    while(--count > 0)
                    {
                        Test.TestIntfPrx[] proxies;
                        if(count == 1)
                        {
                            com.deactivateObjectAdapter(adapters[4]);
                            --adapterCount;
                        }
                        proxies = new Test.TestIntfPrx[10];

                        int i;
                        for(i = 0; i < proxies.Length; ++i)
                        {
                            var adpts = new Test.RemoteObjectAdapterPrx[rand.Next(adapters.Length)];
                            if(adpts.Length == 0)
                            {
                                adpts = new Test.RemoteObjectAdapterPrx[1];
                            }
                            for(int j = 0; j < adpts.Length; ++j)
                            {
                                adpts[j] = adapters[rand.Next(adapters.Length)];
                            }
                            proxies[i] = createTestIntfPrx(new List<Test.RemoteObjectAdapterPrx>(adpts));
                        }

                        for(i = 0; i < proxies.Length; i++)
                        {
                            proxies[i].begin_getAdapterName();
                        }
                        for(i = 0; i < proxies.Length; i++)
                        {
                            try
                            {
                                proxies[i].ice_ping();
                            }
                            catch(Ice.LocalException)
                            {
                            }
                        }

                        List<Ice.Connection> connections = new List<Ice.Connection>();
                        for(i = 0; i < proxies.Length; i++)
                        {
                            if(proxies[i].ice_getCachedConnection() != null)
                            {
                                if(!connections.Contains(proxies[i].ice_getCachedConnection()))
                                {
                                    connections.Add(proxies[i].ice_getCachedConnection());
                                }
                            }
                        }
                        test(connections.Count <= adapterCount);

                        foreach(var a in adapters)
                        {
                            try
                            {
                                a.getTestIntf().ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                            }
                            catch(Ice.LocalException)
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
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("AdapterAMI11", "default"));
                    adapters.Add(com.createObjectAdapter("AdapterAMI12", "default"));
                    adapters.Add(com.createObjectAdapter("AdapterAMI13", "default"));

                    //
                    // Ensure that when a connection is opened it's reused for new
                    // proxies and that all endpoints are eventually tried.
                    //
                    List<string> names = new List<string>();
                    names.Add("AdapterAMI11");
                    names.Add("AdapterAMI12");
                    names.Add("AdapterAMI13");
                    while(names.Count > 0)
                    {
                        var adpts = new List<Test.RemoteObjectAdapterPrx>(adapters);

                        var test1 = createTestIntfPrx(adpts);
                        shuffle(ref adpts);
                        var test2 = createTestIntfPrx(adpts);
                        shuffle(ref adpts);
                        var test3 = createTestIntfPrx(adpts);
                        test1.ice_ping();
                        test(test1.ice_getConnection() == test2.ice_getConnection());
                        test(test2.ice_getConnection() == test3.ice_getConnection());

                        names.Remove(getAdapterNameWithAMI(test1));
                        test1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    }

                    //
                    // Ensure that the proxy correctly caches the connection(we
                    // always send the request over the same connection.)
                    //
                    {
                        foreach(var adpt in adapters)
                        {
                            adpt.getTestIntf().ice_ping();
                        }

                        var t = createTestIntfPrx(adapters);
                        string name = getAdapterNameWithAMI(t);
                        int nRetry = 10;
                        int i;
                        for(i = 0; i < nRetry && getAdapterNameWithAMI(t).Equals(name); i++) ;
                        test(i == nRetry);

                        foreach(var adpt in adapters)
                        {
                            adpt.getTestIntf().ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                        }
                    }

                    //
                    // Deactivate an adapter and ensure that we can still
                    // establish the connection to the remaining adapters.
                    //
                    com.deactivateObjectAdapter(adapters[0]);
                    names.Add("AdapterAMI12");
                    names.Add("AdapterAMI13");
                    while(names.Count > 0)
                    {
                        var adpts = new List<Test.RemoteObjectAdapterPrx>(adapters);

                        var test1 = createTestIntfPrx(adpts);
                        shuffle(ref adpts);
                        var test2 = createTestIntfPrx(adpts);
                        shuffle(ref adpts);
                        var test3 = createTestIntfPrx(adpts);

                        test(test1.ice_getConnection() == test2.ice_getConnection());
                        test(test2.ice_getConnection() == test3.ice_getConnection());

                        names.Remove(getAdapterNameWithAMI(test1));
                        test1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    }

                    //
                    // Deactivate an adapter and ensure that we can still
                    // establish the connection to the remaining adapter.
                    //
                    com.deactivateObjectAdapter(adapters[2]);
                    var obj = createTestIntfPrx(adapters);
                    test(getAdapterNameWithAMI(obj).Equals("AdapterAMI12"));

                    deactivate(com, adapters);
                }
                output.WriteLine("ok");

                output.Write("testing random endpoint selection... ");
                output.Flush();
                {
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("Adapter21", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter22", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter23", "default"));

                    var obj = createTestIntfPrx(adapters);
                    test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

                    var names = new List<string>();
                    names.Add("Adapter21");
                    names.Add("Adapter22");
                    names.Add("Adapter23");
                    while(names.Count > 0)
                    {
                        names.Remove(obj.getAdapterName());
                        obj.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    }

                    obj = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Random));
                    test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

                    names.Add("Adapter21");
                    names.Add("Adapter22");
                    names.Add("Adapter23");
                    while(names.Count > 0)
                    {
                        names.Remove(obj.getAdapterName());
                        obj.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    }

                    deactivate(com, adapters);
                }
                output.WriteLine("ok");

                output.Write("testing ordered endpoint selection... ");
                output.Flush();
                {
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("Adapter31", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter32", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter33", "default"));

                    var obj = createTestIntfPrx(adapters);
                    obj = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
                    test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
                    int nRetry = 3;
                    int i;

                    //
                    // Ensure that endpoints are tried in order by deactiving the adapters
                    // one after the other.
                    //
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter31"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[0]);
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter32"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[1]);
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter33"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[2]);

                    try
                    {
                        obj.getAdapterName();
                    }
                    catch(Ice.ConnectFailedException)
                    {
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                    }

                    Ice.Endpoint[] endpoints = obj.ice_getEndpoints();

                    adapters.Clear();

                    //
                    // Now, re-activate the adapters with the same endpoints in the opposite
                    // order.
                    //
                    adapters.Add(com.createObjectAdapter("Adapter36", endpoints[2].ToString()));
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter36"); i++) ;
                    test(i == nRetry);
                    obj.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    adapters.Add(com.createObjectAdapter("Adapter35", endpoints[1].ToString()));
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter35"); i++) ;
                    test(i == nRetry);
                    obj.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    adapters.Add(com.createObjectAdapter("Adapter34", endpoints[0].ToString()));
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter34"); i++) ;
                    test(i == nRetry);

                    deactivate(com, adapters);
                }
                output.WriteLine("ok");

                output.Write("testing per request binding with single endpoint... ");
                output.Flush();
                {
                    var adapter = com.createObjectAdapter("Adapter41", "default");

                    var test1 = Test.TestIntfPrxHelper.uncheckedCast(adapter.getTestIntf().ice_connectionCached(false));
                    var test2 = Test.TestIntfPrxHelper.uncheckedCast(adapter.getTestIntf().ice_connectionCached(false));
                    test(!test1.ice_isConnectionCached());
                    test(!test2.ice_isConnectionCached());
                    test(test1.ice_getConnection() != null && test2.ice_getConnection() != null);
                    test(test1.ice_getConnection() == test2.ice_getConnection());

                    test1.ice_ping();

                    com.deactivateObjectAdapter(adapter);

                    var test3 = Test.TestIntfPrxHelper.uncheckedCast(test1);
                    try
                    {
                        test(test3.ice_getConnection() == test1.ice_getConnection());
                        test(false);
                    }
                    catch(Ice.ConnectFailedException)
                    {
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                    }
                }
                output.WriteLine("ok");

                output.Write("testing per request binding with multiple endpoints... ");
                output.Flush();
                {
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("Adapter51", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter52", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter53", "default"));

                    var obj = Test.TestIntfPrxHelper.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(false));
                    test(!obj.ice_isConnectionCached());

                    List<string> names = new List<string>();
                    names.Add("Adapter51");
                    names.Add("Adapter52");
                    names.Add("Adapter53");
                    while(names.Count > 0)
                    {
                        names.Remove(obj.getAdapterName());
                    }

                    com.deactivateObjectAdapter(adapters[0]);

                    names.Add("Adapter52");
                    names.Add("Adapter53");
                    while(names.Count > 0)
                    {
                        names.Remove(obj.getAdapterName());
                    }

                    com.deactivateObjectAdapter(adapters[2]);

                    test(obj.getAdapterName().Equals("Adapter52"));

                    deactivate(com, adapters);
                }
                output.WriteLine("ok");

                output.Write("testing per request binding with multiple endpoints and AMI... ");
                output.Flush();
                {
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("AdapterAMI51", "default"));
                    adapters.Add(com.createObjectAdapter("AdapterAMI52", "default"));
                    adapters.Add(com.createObjectAdapter("AdapterAMI53", "default"));

                    var obj = Test.TestIntfPrxHelper.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(false));
                    test(!obj.ice_isConnectionCached());

                    var names = new List<string>();
                    names.Add("AdapterAMI51");
                    names.Add("AdapterAMI52");
                    names.Add("AdapterAMI53");
                    while(names.Count > 0)
                    {
                        names.Remove(getAdapterNameWithAMI(obj));
                    }

                    com.deactivateObjectAdapter(adapters[0]);

                    names.Add("AdapterAMI52");
                    names.Add("AdapterAMI53");
                    while(names.Count > 0)
                    {
                        names.Remove(getAdapterNameWithAMI(obj));
                    }

                    com.deactivateObjectAdapter(adapters[2]);

                    test(getAdapterNameWithAMI(obj).Equals("AdapterAMI52"));

                    deactivate(com, adapters);
                }
                output.WriteLine("ok");

                output.Write("testing per request binding and ordered endpoint selection... ");
                output.Flush();
                {
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("Adapter61", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter62", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter63", "default"));

                    var obj = createTestIntfPrx(adapters);
                    obj = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
                    test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
                    obj = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_connectionCached(false));
                    test(!obj.ice_isConnectionCached());
                    int nRetry = 3;
                    int i;

                    //
                    // Ensure that endpoints are tried in order by deactiving the adapters
                    // one after the other.
                    //
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter61"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[0]);
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter62"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[1]);
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter63"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[2]);

                    try
                    {
                        obj.getAdapterName();
                    }
                    catch(Ice.ConnectFailedException)
                    {
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                    }

                    Ice.Endpoint[] endpoints = obj.ice_getEndpoints();

                    adapters.Clear();

                    //
                    // Now, re-activate the adapters with the same endpoints in the opposite
                    // order.
                    //
                    adapters.Add(com.createObjectAdapter("Adapter66", endpoints[2].ToString()));
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter66"); i++) ;
                    test(i == nRetry);
                    adapters.Add(com.createObjectAdapter("Adapter65", endpoints[1].ToString()));
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter65"); i++) ;
                    test(i == nRetry);
                    adapters.Add(com.createObjectAdapter("Adapter64", endpoints[0].ToString()));
                    for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter64"); i++) ;
                    test(i == nRetry);

                    deactivate(com, adapters);
                }
                output.WriteLine("ok");

                output.Write("testing per request binding and ordered endpoint selection and AMI... ");
                output.Flush();
                {
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("AdapterAMI61", "default"));
                    adapters.Add(com.createObjectAdapter("AdapterAMI62", "default"));
                    adapters.Add(com.createObjectAdapter("AdapterAMI63", "default"));

                    var obj = createTestIntfPrx(adapters);
                    obj = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
                    test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
                    obj = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_connectionCached(false));
                    test(!obj.ice_isConnectionCached());
                    int nRetry = 3;
                    int i;

                    //
                    // Ensure that endpoints are tried in order by deactiving the adapters
                    // one after the other.
                    //
                    for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI61"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[0]);
                    for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI62"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[1]);
                    for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI63"); i++) ;
                    test(i == nRetry);
                    com.deactivateObjectAdapter(adapters[2]);

                    try
                    {
                        obj.getAdapterName();
                    }
                    catch(Ice.ConnectFailedException)
                    {
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                    }

                    Ice.Endpoint[] endpoints = obj.ice_getEndpoints();

                    adapters.Clear();

                    //
                    // Now, re-activate the adapters with the same endpoints in the opposite
                    // order.
                    //
                    adapters.Add(com.createObjectAdapter("AdapterAMI66", endpoints[2].ToString()));
                    for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI66"); i++) ;
                    test(i == nRetry);
                    adapters.Add(com.createObjectAdapter("AdapterAMI65", endpoints[1].ToString()));
                    for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI65"); i++) ;
                    test(i == nRetry);
                    adapters.Add(com.createObjectAdapter("AdapterAMI64", endpoints[0].ToString()));
                    for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI64"); i++) ;
                    test(i == nRetry);

                    deactivate(com, adapters);
                }
                output.WriteLine("ok");

                output.Write("testing endpoint mode filtering... ");
                output.Flush();
                {
                    var adapters = new List<Test.RemoteObjectAdapterPrx>();
                    adapters.Add(com.createObjectAdapter("Adapter71", "default"));
                    adapters.Add(com.createObjectAdapter("Adapter72", "udp"));

                    var obj = createTestIntfPrx(adapters);
                    test(obj.getAdapterName().Equals("Adapter71"));

                    var testUDP = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_datagram());
                    test(obj.ice_getConnection() != testUDP.ice_getConnection());
                    try
                    {
                        testUDP.getAdapterName();
                    }
                    catch(Ice.TwowayOnlyException)
                    {
                    }
                }
                output.WriteLine("ok");
                if(communicator.getProperties().getProperty("Ice.Plugin.IceSSL").Length > 0)
                {
                    output.Write("testing unsecure vs. secure endpoints... ");
                    output.Flush();
                    {
                        var adapters = new List<Test.RemoteObjectAdapterPrx>();
                        adapters.Add(com.createObjectAdapter("Adapter81", "ssl"));
                        adapters.Add(com.createObjectAdapter("Adapter82", "tcp"));

                        var obj = createTestIntfPrx(adapters);
                        int i;
                        for(i = 0; i < 5; i++)
                        {
                            test(obj.getAdapterName().Equals("Adapter82"));
                            obj.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                        }

                        var testSecure = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_secure(true));
                        test(testSecure.ice_isSecure());
                        testSecure = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_secure(false));
                        test(!testSecure.ice_isSecure());
                        testSecure = Test.TestIntfPrxHelper.uncheckedCast(obj.ice_secure(true));
                        test(testSecure.ice_isSecure());
                        test(obj.ice_getConnection() != testSecure.ice_getConnection());

                        com.deactivateObjectAdapter(adapters[1]);

                        for(i = 0; i < 5; i++)
                        {
                            test(obj.getAdapterName().Equals("Adapter81"));
                            obj.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                        }

                        com.createObjectAdapter("Adapter83",(obj.ice_getEndpoints()[1]).ToString()); // Reactive tcp OA.

                        for(i = 0; i < 5; i++)
                        {
                            test(obj.getAdapterName().Equals("Adapter83"));
                            obj.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                        }

                        com.deactivateObjectAdapter(adapters[0]);
                        try
                        {
                            testSecure.ice_ping();
                            test(false);
                        }
                        catch(Ice.ConnectFailedException)
                        {
                        }
                        catch(Ice.ConnectTimeoutException)
                        {
                        }

                        deactivate(com, adapters);
                    }
                    output.WriteLine("ok");
                }

                {
                    output.Write("testing ipv4 & ipv6 connections... ");
                    output.Flush();

                    Ice.Properties ipv4 = Ice.Util.createProperties();
                    ipv4.setProperty("Ice.IPv4", "1");
                    ipv4.setProperty("Ice.IPv6", "0");
                    ipv4.setProperty("Adapter.Endpoints", "tcp -h localhost");

                    Ice.Properties ipv6 = Ice.Util.createProperties();
                    ipv6.setProperty("Ice.IPv4", "0");
                    ipv6.setProperty("Ice.IPv6", "1");
                    ipv6.setProperty("Adapter.Endpoints", "tcp -h localhost");

                    Ice.Properties bothPreferIPv4 = Ice.Util.createProperties();
                    bothPreferIPv4.setProperty("Ice.IPv4", "1");
                    bothPreferIPv4.setProperty("Ice.IPv6", "1");
                    bothPreferIPv4.setProperty("Ice.PreferIPv6Address", "0");
                    bothPreferIPv4.setProperty("Adapter.Endpoints", "tcp -h localhost");

                    Ice.Properties bothPreferIPv6 = Ice.Util.createProperties();
                    bothPreferIPv6.setProperty("Ice.IPv4", "1");
                    bothPreferIPv6.setProperty("Ice.IPv6", "1");
                    bothPreferIPv6.setProperty("Ice.PreferIPv6Address", "1");
                    bothPreferIPv6.setProperty("Adapter.Endpoints", "tcp -h localhost");

                    List<Ice.Properties> clientProps = new List<Ice.Properties>();
                    clientProps.Add(ipv4);
                    clientProps.Add(ipv6);
                    clientProps.Add(bothPreferIPv4);
                    clientProps.Add(bothPreferIPv6);

                    string endpoint = "tcp -p " + helper.getTestPort(2).ToString();

                    Ice.Properties anyipv4 = ipv4.ice_clone_();
                    anyipv4.setProperty("Adapter.Endpoints", endpoint);
                    anyipv4.setProperty("Adapter.PublishedEndpoints", endpoint + " -h 127.0.0.1");

                    Ice.Properties anyipv6 = ipv6.ice_clone_();
                    anyipv6.setProperty("Adapter.Endpoints", endpoint);
                    anyipv6.setProperty("Adapter.PublishedEndpoints", endpoint + " -h \".1\"");

                    Ice.Properties anyboth = Ice.Util.createProperties();
                    anyboth.setProperty("Ice.IPv4", "1");
                    anyboth.setProperty("Ice.IPv6", "1");
                    anyboth.setProperty("Adapter.Endpoints", endpoint);
                    anyboth.setProperty("Adapter.PublishedEndpoints", endpoint + " -h \"::1\":" + endpoint + " -h 127.0.0.1");

                    Ice.Properties localipv4 = ipv4.ice_clone_();
                    localipv4.setProperty("Adapter.Endpoints", "tcp -h 127.0.0.1");

                    Ice.Properties localipv6 = ipv6.ice_clone_();
                    localipv6.setProperty("Adapter.Endpoints", "tcp -h \"::1\"");

                    List<Ice.Properties> serverProps = new List<Ice.Properties>(clientProps);
                    serverProps.Add(anyipv4);
                    serverProps.Add(anyipv6);
                    serverProps.Add(anyboth);
                    serverProps.Add(localipv4);
                    serverProps.Add(localipv6);

                    bool ipv6NotSupported = false;
                    foreach(Ice.Properties p in serverProps)
                    {
                        Ice.InitializationData serverInitData = new Ice.InitializationData();
                        serverInitData.properties = p;
                        Ice.Communicator serverCommunicator = Ice.Util.initialize(serverInitData);
                        Ice.ObjectAdapter oa;
                        try
                        {
                            oa = serverCommunicator.createObjectAdapter("Adapter");
                            oa.activate();
                        }
                        catch(Ice.DNSException)
                        {
                            serverCommunicator.destroy();
                            continue; // IP version not supported.
                        }
                        catch(Ice.SocketException)
                        {
                            if(p == ipv6)
                            {
                                ipv6NotSupported = true;
                            }
                            serverCommunicator.destroy();
                            continue; // IP version not supported.
                        }

                        Ice.ObjectPrx prx = oa.createProxy(Ice.Util.stringToIdentity("dummy"));
                        try
                        {
                            prx.ice_collocationOptimized(false).ice_ping();
                        }
                        catch(Ice.LocalException)
                        {
                            serverCommunicator.destroy();
                            continue; // IP version not supported.
                        }

                        string strPrx = prx.ToString();
                        foreach(Ice.Properties q in clientProps)
                        {
                            Ice.InitializationData clientInitData = new Ice.InitializationData();
                            clientInitData.properties = q;
                            Ice.Communicator clientCommunicator = Ice.Util.initialize(clientInitData);
                            prx = clientCommunicator.stringToProxy(strPrx);
                            try
                            {
                                prx.ice_ping();
                                test(false);
                            }
                            catch(Ice.ObjectNotExistException)
                            {
                                // Expected, no object registered.
                            }
                            catch(Ice.DNSException)
                            {
                                // Expected if no IPv4 or IPv6 address is
                                // associated to localhost or if trying to connect
                                // to an any endpoint with the wrong IP version,
                                // e.g.: resolving an IPv4 address when only IPv6
                                // is enabled fails with a DNS exception.
                            }
                            catch(Ice.SocketException)
                            {
                                test((p == ipv4 && q == ipv6) ||(p == ipv6 && q == ipv4) ||
                                    (p == bothPreferIPv4 && q == ipv6) ||(p == bothPreferIPv6 && q == ipv4) ||
                                    (p == bothPreferIPv6 && q == ipv6 && ipv6NotSupported) ||
                                    (p == anyipv4 && q == ipv6) ||(p == anyipv6 && q == ipv4) ||
                                    (p == localipv4 && q == ipv6) ||(p == localipv6 && q == ipv4) ||
                                    (p == ipv6 && q == bothPreferIPv4) ||(p == ipv6 && q == bothPreferIPv6) ||
                                    (p == bothPreferIPv6 && q == ipv6));
                            }
                            clientCommunicator.destroy();
                        }
                        serverCommunicator.destroy();
                    }

                    output.WriteLine("ok");
                }
                com.shutdown();
            }

            private static Random _rand = new Random(unchecked((int)DateTime.Now.Ticks));
        }
    }
}
