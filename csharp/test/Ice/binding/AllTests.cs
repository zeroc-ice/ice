// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using Test;

public class AllTests : TestCommon.TestApp
{
    private static string getAdapterNameWithAMI(TestIntfPrx testIntf)
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

    private static void shuffle(ref List<RemoteObjectAdapterPrx> array)
    {
        for (int i = 0; i < array.Count - 1; ++i)
        {
            int r = rand_.Next(array.Count - i) + i;
            Debug.Assert(r >= i && r < array.Count);
            if (r != i)
            {
                RemoteObjectAdapterPrx tmp = array[i];
                array[i] = array[r];
                array[r] = tmp;
            }
        }
    }

    private static TestIntfPrx createTestIntfPrx(List<RemoteObjectAdapterPrx> adapters)
    {
        List<Ice.Endpoint> endpoints = new List<Ice.Endpoint>();
        TestIntfPrx obj = null;
        IEnumerator<RemoteObjectAdapterPrx> p = adapters.GetEnumerator();
        while(p.MoveNext())
        {
            obj = p.Current.getTestIntf();
            foreach(Ice.Endpoint e in obj.ice_getEndpoints())
            {
                endpoints.Add(e);
            }
        }
        return TestIntfPrxHelper.uncheckedCast(obj.ice_endpoints(endpoints.ToArray()));
    }

    private static void deactivate(RemoteCommunicatorPrx communicator, List<RemoteObjectAdapterPrx> adapters)
    {
        IEnumerator<RemoteObjectAdapterPrx> p = adapters.GetEnumerator();
        while (p.MoveNext())
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

    public static void allTests(Ice.Communicator communicator)
    {
        string @ref = "communicator:default -p 12010";
        RemoteCommunicatorPrx com = RemoteCommunicatorPrxHelper.uncheckedCast(communicator.stringToProxy(@ref));

        System.Random rand = new System.Random(unchecked((int)System.DateTime.Now.Ticks));

        Write("testing binding with single endpoint... ");
        Flush();
        {
            RemoteObjectAdapterPrx adapter = com.createObjectAdapter("Adapter", "default");

            TestIntfPrx test1 = adapter.getTestIntf();
            TestIntfPrx test2 = adapter.getTestIntf();
            test(test1.ice_getConnection() == test2.ice_getConnection());

            test1.ice_ping();
            test2.ice_ping();

            com.deactivateObjectAdapter(adapter);

            TestIntfPrx test3 = TestIntfPrxHelper.uncheckedCast(test1);
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
        }
        WriteLine("ok");

        Write("testing binding with multiple endpoints... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
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
                List<RemoteObjectAdapterPrx> adpts = new List<RemoteObjectAdapterPrx>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                shuffle(ref adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                shuffle(ref adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);
                test1.ice_ping();
                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.Remove(test1.getAdapterName());
                test1.ice_getConnection().close(false);
            }

            //
            // Ensure that the proxy correctly caches the connection (we
            // always send the request over the same connection.)
            //
            {
                foreach(RemoteObjectAdapterPrx adpt in adapters)
                {
                    adpt.getTestIntf().ice_ping();
                }

                TestIntfPrx t = createTestIntfPrx(adapters);
                string name = t.getAdapterName();
                int nRetry = 10;
                int i;
                for(i = 0; i < nRetry && t.getAdapterName().Equals(name); i++);
                test(i == nRetry);

                foreach(RemoteObjectAdapterPrx adpt in adapters)
                {
                    adpt.getTestIntf().ice_getConnection().close(false);
                }
            }

            //
            // Deactivate an adapter and ensure that we can still
            // establish the connection to the remaining adapters.
            //
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[0]);
            names.Add("Adapter12");
            names.Add("Adapter13");
            while(names.Count > 0)
            {
                List<RemoteObjectAdapterPrx> adpts = new List<RemoteObjectAdapterPrx>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                shuffle(ref adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                shuffle(ref adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.Remove(test1.getAdapterName());
                test1.ice_getConnection().close(false);
            }

            //
            // Deactivate an adapter and ensure that we can still
            // establish the connection to the remaining adapter.
            //
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[2]);
            TestIntfPrx obj = createTestIntfPrx(adapters);
            test(obj.getAdapterName().Equals("Adapter12"));

            deactivate(com, adapters);
        }
        WriteLine("ok");

        Write("testing binding with multiple random endpoints... ");
        Flush();
        {
            RemoteObjectAdapterPrx[] adapters = new RemoteObjectAdapterPrx[5];
            adapters[0] = com.createObjectAdapter("AdapterRandom11", "default");
            adapters[1] = com.createObjectAdapter("AdapterRandom12", "default");
            adapters[2] = com.createObjectAdapter("AdapterRandom13", "default");
            adapters[3] = com.createObjectAdapter("AdapterRandom14", "default");
            adapters[4] = com.createObjectAdapter("AdapterRandom15", "default");

            int count;
            if(IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows)
            {
                count = 20;
            }
            else
            {
                count = 60;
            }

            int adapterCount = adapters.Length;
            while(--count > 0)
            {
                TestIntfPrx[] proxies;
                if(IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows)
                {
                    if(count == 10)
                    {
                        com.deactivateObjectAdapter(adapters[4]);
                        --adapterCount;
                    }
                    proxies = new TestIntfPrx[10];
                }
                else
                {
                    if(count < 60 && count % 10 == 0)
                    {
                        com.deactivateObjectAdapter(adapters[count / 10 - 1]);
                        --adapterCount;
                    }
                    proxies = new TestIntfPrx[40];
                }

                int i;
                for(i = 0; i < proxies.Length; ++i)
                {
                    RemoteObjectAdapterPrx[] adpts = new RemoteObjectAdapterPrx[rand.Next(adapters.Length)];
                    if(adpts.Length == 0)
                    {
                        adpts = new RemoteObjectAdapterPrx[1];
                    }
                    for(int j = 0; j < adpts.Length; ++j)
                    {
                        adpts[j] = adapters[rand.Next(adapters.Length)];
                    }
                    proxies[i] = createTestIntfPrx(new List<RemoteObjectAdapterPrx>(adpts));
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

                foreach(RemoteObjectAdapterPrx a in adapters)
                {
                    try
                    {
                        a.getTestIntf().ice_getConnection().close(false);
                    }
                    catch(Ice.LocalException)
                    {
                        // Expected if adapter is down.
                    }
                }
            }
        }
        WriteLine("ok");

        Write("testing binding with multiple endpoints and AMI... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
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
                List<RemoteObjectAdapterPrx> adpts = new List<RemoteObjectAdapterPrx>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                shuffle(ref adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                shuffle(ref adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);
                test1.ice_ping();
                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.Remove(getAdapterNameWithAMI(test1));
                test1.ice_getConnection().close(false);
            }

            //
            // Ensure that the proxy correctly caches the connection (we
            // always send the request over the same connection.)
            //
            {
                foreach(RemoteObjectAdapterPrx adpt in adapters)
                {
                    adpt.getTestIntf().ice_ping();
                }

                TestIntfPrx t = createTestIntfPrx(adapters);
                string name = getAdapterNameWithAMI(t);
                int nRetry = 10;
                int i;
                for(i = 0; i < nRetry && getAdapterNameWithAMI(t).Equals(name); i++);
                test(i == nRetry);

                foreach(RemoteObjectAdapterPrx adpt in adapters)
                {
                    adpt.getTestIntf().ice_getConnection().close(false);
                }
            }

            //
            // Deactivate an adapter and ensure that we can still
            // establish the connection to the remaining adapters.
            //
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[0]);
            names.Add("AdapterAMI12");
            names.Add("AdapterAMI13");
            while(names.Count > 0)
            {
                List<RemoteObjectAdapterPrx> adpts = new List<RemoteObjectAdapterPrx>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                shuffle(ref adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                shuffle(ref adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.Remove(getAdapterNameWithAMI(test1));
                test1.ice_getConnection().close(false);
            }

            //
            // Deactivate an adapter and ensure that we can still
            // establish the connection to the remaining adapter.
            //
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[2]);
            TestIntfPrx obj = createTestIntfPrx(adapters);
            test(getAdapterNameWithAMI(obj).Equals("AdapterAMI12"));

            deactivate(com, adapters);
        }
       WriteLine("ok");

        Write("testing random endpoint selection... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
            adapters.Add(com.createObjectAdapter("Adapter21", "default"));
            adapters.Add(com.createObjectAdapter("Adapter22", "default"));
            adapters.Add(com.createObjectAdapter("Adapter23", "default"));

            TestIntfPrx obj = createTestIntfPrx(adapters);
            test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

            List<string> names = new List<string>();
            names.Add("Adapter21");
            names.Add("Adapter22");
            names.Add("Adapter23");
            while(names.Count > 0)
            {
                names.Remove(obj.getAdapterName());
                obj.ice_getConnection().close(false);
            }

            obj = TestIntfPrxHelper.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Random));
            test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

            names.Add("Adapter21");
            names.Add("Adapter22");
            names.Add("Adapter23");
            while(names.Count > 0)
            {
                names.Remove(obj.getAdapterName());
                obj.ice_getConnection().close(false);
            }

            deactivate(com, adapters);
        }
        WriteLine("ok");

        Write("testing ordered endpoint selection... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
            adapters.Add(com.createObjectAdapter("Adapter31", "default"));
            adapters.Add(com.createObjectAdapter("Adapter32", "default"));
            adapters.Add(com.createObjectAdapter("Adapter33", "default"));

            TestIntfPrx obj = createTestIntfPrx(adapters);
            obj = TestIntfPrxHelper.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
            test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            int nRetry = 3;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter31"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[0]);
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter32"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[1]);
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter33"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[2]);

            try
            {
                obj.getAdapterName();
            }
            catch(Ice.ConnectFailedException)
            {
            }

            Ice.Endpoint[] endpoints = obj.ice_getEndpoints();

            adapters.Clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.Add(com.createObjectAdapter("Adapter36", endpoints[2].ToString()));
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter36"); i++);
            test(i == nRetry);
            obj.ice_getConnection().close(false);
            adapters.Add(com.createObjectAdapter("Adapter35", endpoints[1].ToString()));
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter35"); i++);
            test(i == nRetry);
            obj.ice_getConnection().close(false);
            adapters.Add(com.createObjectAdapter("Adapter34", endpoints[0].ToString()));
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter34"); i++);
            test(i == nRetry);

            deactivate(com, adapters);
        }
        WriteLine("ok");

        Write("testing per request binding with single endpoint... ");
        Flush();
        {
            RemoteObjectAdapterPrx adapter = com.createObjectAdapter("Adapter41", "default");

            TestIntfPrx test1 = TestIntfPrxHelper.uncheckedCast(adapter.getTestIntf().ice_connectionCached(false));
            TestIntfPrx test2 = TestIntfPrxHelper.uncheckedCast(adapter.getTestIntf().ice_connectionCached(false));
            test(!test1.ice_isConnectionCached());
            test(!test2.ice_isConnectionCached());
            test(test1.ice_getConnection() == test2.ice_getConnection());

            test1.ice_ping();

            com.deactivateObjectAdapter(adapter);

            TestIntfPrx test3 = TestIntfPrxHelper.uncheckedCast(test1);
            try
            {
                test(test3.ice_getConnection() == test1.ice_getConnection());
                test(false);
            }
            catch(Ice.ConnectFailedException)
            {
            }
        }
        WriteLine("ok");

        Write("testing per request binding with multiple endpoints... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
            adapters.Add(com.createObjectAdapter("Adapter51", "default"));
            adapters.Add(com.createObjectAdapter("Adapter52", "default"));
            adapters.Add(com.createObjectAdapter("Adapter53", "default"));

            TestIntfPrx obj = TestIntfPrxHelper.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(false));
            test(!obj.ice_isConnectionCached());

            List<string> names = new List<string>();
            names.Add("Adapter51");
            names.Add("Adapter52");
            names.Add("Adapter53");
            while(names.Count > 0)
            {
                names.Remove(obj.getAdapterName());
            }

            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[0]);

            names.Add("Adapter52");
            names.Add("Adapter53");
            while(names.Count > 0)
            {
                names.Remove(obj.getAdapterName());
            }

            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[2]);


            test(obj.getAdapterName().Equals("Adapter52"));

            deactivate(com, adapters);
        }
        WriteLine("ok");

        Write("testing per request binding with multiple endpoints and AMI... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
            adapters.Add(com.createObjectAdapter("AdapterAMI51", "default"));
            adapters.Add(com.createObjectAdapter("AdapterAMI52", "default"));
            adapters.Add(com.createObjectAdapter("AdapterAMI53", "default"));

            TestIntfPrx obj = TestIntfPrxHelper.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(false));
            test(!obj.ice_isConnectionCached());

            List<string> names = new List<string>();
            names.Add("AdapterAMI51");
            names.Add("AdapterAMI52");
            names.Add("AdapterAMI53");
            while(names.Count > 0)
            {
                names.Remove(getAdapterNameWithAMI(obj));
            }

            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[0]);

            names.Add("AdapterAMI52");
            names.Add("AdapterAMI53");
            while(names.Count > 0)
            {
                names.Remove(getAdapterNameWithAMI(obj));
            }

            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[2]);


            test(getAdapterNameWithAMI(obj).Equals("AdapterAMI52"));

            deactivate(com, adapters);
        }
        WriteLine("ok");

        Write("testing per request binding and ordered endpoint selection... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
            adapters.Add(com.createObjectAdapter("Adapter61", "default"));
            adapters.Add(com.createObjectAdapter("Adapter62", "default"));
            adapters.Add(com.createObjectAdapter("Adapter63", "default"));

            TestIntfPrx obj = createTestIntfPrx(adapters);
            obj = TestIntfPrxHelper.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
            test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            obj = TestIntfPrxHelper.uncheckedCast(obj.ice_connectionCached(false));
            test(!obj.ice_isConnectionCached());
            int nRetry = 3;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter61"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[0]);
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter62"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[1]);
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter63"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[2]);

            try
            {
                obj.getAdapterName();
            }
            catch(Ice.ConnectFailedException)
            {
            }

            Ice.Endpoint[] endpoints = obj.ice_getEndpoints();

            adapters.Clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.Add(com.createObjectAdapter("Adapter66", endpoints[2].ToString()));
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter66"); i++);
            test(i == nRetry);
            adapters.Add(com.createObjectAdapter("Adapter65", endpoints[1].ToString()));
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter65"); i++);
            test(i == nRetry);
            adapters.Add(com.createObjectAdapter("Adapter64", endpoints[0].ToString()));
            for(i = 0; i < nRetry && obj.getAdapterName().Equals("Adapter64"); i++);
            test(i == nRetry);

            deactivate(com, adapters);
        }
        WriteLine("ok");

        Write("testing per request binding and ordered endpoint selection and AMI... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
            adapters.Add(com.createObjectAdapter("AdapterAMI61", "default"));
            adapters.Add(com.createObjectAdapter("AdapterAMI62", "default"));
            adapters.Add(com.createObjectAdapter("AdapterAMI63", "default"));

            TestIntfPrx obj = createTestIntfPrx(adapters);
            obj = TestIntfPrxHelper.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
            test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            obj = TestIntfPrxHelper.uncheckedCast(obj.ice_connectionCached(false));
            test(!obj.ice_isConnectionCached());
            int nRetry = 3;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI61"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[0]);
            for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI62"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[1]);
            for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI63"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[2]);

            try
            {
                obj.getAdapterName();
            }
            catch(Ice.ConnectFailedException)
            {
            }

            Ice.Endpoint[] endpoints = obj.ice_getEndpoints();

            adapters.Clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.Add(com.createObjectAdapter("AdapterAMI66", endpoints[2].ToString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI66"); i++);
            test(i == nRetry);
            adapters.Add(com.createObjectAdapter("AdapterAMI65", endpoints[1].ToString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI65"); i++);
            test(i == nRetry);
            adapters.Add(com.createObjectAdapter("AdapterAMI64", endpoints[0].ToString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(obj).Equals("AdapterAMI64"); i++);
            test(i == nRetry);

            deactivate(com, adapters);
        }
        WriteLine("ok");

        Write("testing endpoint mode filtering... ");
        Flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
            adapters.Add(com.createObjectAdapter("Adapter71", "default"));
            adapters.Add(com.createObjectAdapter("Adapter72", "udp"));

            TestIntfPrx obj = createTestIntfPrx(adapters);
            test(obj.getAdapterName().Equals("Adapter71"));

            TestIntfPrx testUDP = TestIntfPrxHelper.uncheckedCast(obj.ice_datagram());
            test(obj.ice_getConnection() != testUDP.ice_getConnection());
            try
            {
                testUDP.getAdapterName();
            }
            catch(System.ArgumentException)
            {
            }
        }
        WriteLine("ok");
        if(communicator.getProperties().getProperty("Ice.Plugin.IceSSL").Length > 0)
        {
            Write("testing unsecure vs. secure endpoints... ");
            Flush();
            {
                List<RemoteObjectAdapterPrx> adapters = new List<RemoteObjectAdapterPrx>();
                adapters.Add(com.createObjectAdapter("Adapter81", "ssl"));
                adapters.Add(com.createObjectAdapter("Adapter82", "tcp"));

                TestIntfPrx obj = createTestIntfPrx(adapters);
                int i;
                for(i = 0; i < 5; i++)
                {
                    test(obj.getAdapterName().Equals("Adapter82"));
                    obj.ice_getConnection().close(false);
                }

                TestIntfPrx testSecure = TestIntfPrxHelper.uncheckedCast(obj.ice_secure(true));
                test(testSecure.ice_isSecure());
                testSecure = TestIntfPrxHelper.uncheckedCast(obj.ice_secure(false));
                test(!testSecure.ice_isSecure());
                testSecure = TestIntfPrxHelper.uncheckedCast(obj.ice_secure(true));
                test(testSecure.ice_isSecure());
                test(obj.ice_getConnection() != testSecure.ice_getConnection());

                com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[1]);

                for(i = 0; i < 5; i++)
                {
                    test(obj.getAdapterName().Equals("Adapter81"));
                    obj.ice_getConnection().close(false);
                }

                com.createObjectAdapter("Adapter83", (obj.ice_getEndpoints()[1]).ToString()); // Reactive tcp OA.

                for(i = 0; i < 5; i++)
                {
                    test(obj.getAdapterName().Equals("Adapter83"));
                    obj.ice_getConnection().close(false);
                }

                com.deactivateObjectAdapter((RemoteObjectAdapterPrx)adapters[0]);
                try
                {
                    testSecure.ice_ping();
                    test(false);
                }
                catch(Ice.ConnectFailedException)
                {
                }

                deactivate(com, adapters);
            }
            WriteLine("ok");
        }

        {
            Write("testing ipv4 & ipv6 connections... ");
            Flush();

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

            Ice.Properties anyipv4 = ipv4.ice_clone_();
            anyipv4.setProperty("Adapter.Endpoints", "tcp -p 12012");
            anyipv4.setProperty("Adapter.PublishedEndpoints", "tcp -h 127.0.0.1 -p 12012");

            Ice.Properties anyipv6 = ipv6.ice_clone_();
            anyipv6.setProperty("Adapter.Endpoints", "tcp -p 12012");
            anyipv6.setProperty("Adapter.PublishedEndpoints", "tcp -h \".1\" -p 12012");

            Ice.Properties anyboth = Ice.Util.createProperties();
            anyboth.setProperty("Ice.IPv4", "1");
            anyboth.setProperty("Ice.IPv6", "1");
            anyboth.setProperty("Adapter.Endpoints", "tcp -p 12012");
            anyboth.setProperty("Adapter.PublishedEndpoints", "tcp -h \"::1\" -p 12012:tcp -h 127.0.0.1 -p 12012");

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

                Ice.ObjectPrx prx = oa.createProxy(serverCommunicator.stringToIdentity("dummy"));
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
                        test((p == ipv4 && q == ipv6) || (p == ipv6 && q == ipv4) ||
                             (p == bothPreferIPv4 && q == ipv6) || (p == bothPreferIPv6 && q == ipv4) ||
                             (p == bothPreferIPv6 && q == ipv6 && ipv6NotSupported) ||
                             (p == anyipv4 && q == ipv6) || (p == anyipv6 && q == ipv4) ||
                             (p == localipv4 && q == ipv6) || (p == localipv6 && q == ipv4) ||
                             (p == ipv6 && q == bothPreferIPv4) || (p == ipv6 && q == bothPreferIPv6) ||
                             (p == bothPreferIPv6 && q == ipv6));
                    }
                    clientCommunicator.destroy();
                }
                serverCommunicator.destroy();
            }

            WriteLine("ok");
        }
        com.shutdown();
    }

    private static System.Random rand_ = new System.Random(unchecked((int)System.DateTime.Now.Ticks));
}
