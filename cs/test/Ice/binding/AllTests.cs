// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Diagnostics;
using System.Threading;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    private class GetAdapterNameCB : AMI_TestIntf_getAdapterName
    {
        public override void ice_response(string name)
        {
            lock(this)
            {
                _name = name;
                Monitor.Pulse(this);
            }
        }
        
        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }
        
        public string getResult()
        {
            lock(this)
            {
                while(_name == null)
                {
                    Monitor.Wait(this);
                }
                return _name;
            }       
        }
        
        private string _name = null;
    };

    private static string getAdapterNameWithAMI(TestIntfPrx test)
    {
        GetAdapterNameCB cb = new GetAdapterNameCB();
        test.getAdapterName_async(cb);
        return cb.getResult();
    }

    private static void shuffle(ref ArrayList array)
    {
        for(int i = 0; i < array.Count - 1; ++i)
        {
            int r = rand_.Next(array.Count - i) + i;
            Debug.Assert(r >= i && r < array.Count);
            if(r != i)
            {
                object tmp = array[i];
                array[i] = array[r];
                array[r] = tmp;
            }
        }
    }
    
    private static TestIntfPrx createTestIntfPrx(ArrayList adapters)
    {
        ArrayList endpoints = new ArrayList();
        TestIntfPrx obj = null;
        IEnumerator p = adapters.GetEnumerator();
        while(p.MoveNext())
        {
            obj = ((RemoteObjectAdapterPrx)p.Current).getTestIntf();
            endpoints.AddRange(ArrayList.Adapter(obj.ice_getEndpoints()));
        }
        return TestIntfPrxHelper.uncheckedCast(
            obj.ice_endpoints((Ice.Endpoint[])endpoints.ToArray(typeof(Ice.Endpoint))));
    }

    private static void deactivate(RemoteCommunicatorPrx communicator, ArrayList adapters)
    {
        IEnumerator p = adapters.GetEnumerator();
        while(p.MoveNext())
        {
            communicator.deactivateObjectAdapter((RemoteObjectAdapterPrx)p.Current);
        }
    }

        private class StringComparator : IComparer
        {
            public int Compare(object l, object r)
            {
                return ((string)l).CompareTo((string)r);
            }
        };

    public static void allTests(Ice.Communicator communicator)
    {
        string @ref = "communicator:default -p 12010 -t 10000";
        RemoteCommunicatorPrx com = RemoteCommunicatorPrxHelper.uncheckedCast(communicator.stringToProxy(@ref));

        Console.Out.Write("testing binding with single endpoint... ");
        Console.Out.Flush();
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
            catch(Ice.ConnectionRefusedException)
            {
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing binding with multiple endpoints... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
            adapters.Add(com.createObjectAdapter("Adapter11", "default"));
            adapters.Add(com.createObjectAdapter("Adapter12", "default"));
            adapters.Add(com.createObjectAdapter("Adapter13", "default"));

            //
            // Ensure that when a connection is opened it's reused for new
            // proxies and that all endpoints are eventually tried.
            //
            IceUtil.Set names = new IceUtil.Set();
            names.Add("Adapter11");
            names.Add("Adapter12");
            names.Add("Adapter13");
            while(names.Count > 0)
            {
                ArrayList adpts = new ArrayList(adapters);

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
                ArrayList adpts = new ArrayList(adapters);

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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing binding with multiple endpoints and AMI... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
            adapters.Add(com.createObjectAdapter("AdapterAMI11", "default"));
            adapters.Add(com.createObjectAdapter("AdapterAMI12", "default"));
            adapters.Add(com.createObjectAdapter("AdapterAMI13", "default"));

            //
            // Ensure that when a connection is opened it's reused for new
            // proxies and that all endpoints are eventually tried.
            //
            IceUtil.Set names = new IceUtil.Set();
            names.Add("AdapterAMI11");
            names.Add("AdapterAMI12");
            names.Add("AdapterAMI13");
            while(names.Count > 0)
            {
                ArrayList adpts = new ArrayList(adapters);

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
                ArrayList adpts = new ArrayList(adapters);

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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing random endpoint selection... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
            adapters.Add(com.createObjectAdapter("Adapter21", "default"));
            adapters.Add(com.createObjectAdapter("Adapter22", "default"));
            adapters.Add(com.createObjectAdapter("Adapter23", "default"));

            TestIntfPrx obj = createTestIntfPrx(adapters);
            test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

            IceUtil.Set names = new IceUtil.Set();
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing ordered endpoint selection... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
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
            catch(Ice.ConnectionRefusedException)
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing per request binding with single endpoint... ");
        Console.Out.Flush();
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
            catch(Ice.ConnectionRefusedException)
            {
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing per request binding with multiple endpoints... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
            adapters.Add(com.createObjectAdapter("Adapter51", "default"));
            adapters.Add(com.createObjectAdapter("Adapter52", "default"));
            adapters.Add(com.createObjectAdapter("Adapter53", "default"));

            TestIntfPrx obj = TestIntfPrxHelper.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(false));
            test(!obj.ice_isConnectionCached());

            IceUtil.Set names = new IceUtil.Set();
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing per request binding with multiple endpoints and AMI... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
            adapters.Add(com.createObjectAdapter("AdapterAMI51", "default"));
            adapters.Add(com.createObjectAdapter("AdapterAMI52", "default"));
            adapters.Add(com.createObjectAdapter("AdapterAMI53", "default"));

            TestIntfPrx obj = TestIntfPrxHelper.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(false));
            test(!obj.ice_isConnectionCached());

            IceUtil.Set names = new IceUtil.Set();
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing per request binding and ordered endpoint selection... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
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
            catch(Ice.ConnectionRefusedException)
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing per request binding and ordered endpoint selection and AMI... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
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
            catch(Ice.ConnectionRefusedException)
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing endpoint mode filtering... ");
        Console.Out.Flush();
        {
            ArrayList adapters = new ArrayList();
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
            catch(Ice.TwowayOnlyException)
            {
            }
        }
        Console.Out.WriteLine("ok");

        if(communicator.getProperties().getProperty("Ice.Plugin.IceSSL").Length > 0)
        {
            Console.Out.Write("testing unsecure vs. secure endpoints... ");
            Console.Out.Flush();
            {
                ArrayList adapters = new ArrayList();
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
                catch(Ice.ConnectionRefusedException)
                {
                }

                deactivate(com, adapters);
            }
            Console.Out.WriteLine("ok");
        }

        com.shutdown(); 
    }

    private static System.Random rand_ = new System.Random(unchecked((int)System.DateTime.Now.Ticks));
}
