// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;

import java.io.PrintWriter;

import test.Ice.binding.Test.RemoteCommunicatorPrx;
import test.Ice.binding.Test.RemoteCommunicatorPrxHelper;
import test.Ice.binding.Test.RemoteObjectAdapterPrx;
import test.Ice.binding.Test.TestIntfPrx;
import test.Ice.binding.Test.TestIntfPrxHelper;
import test.Util.Application;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class GetAdapterNameCB extends Ice.Callback
    {
        @Override
        synchronized public void
        completed(Ice.AsyncResult result)
        {
            try
            {
                _name = TestIntfPrxHelper.uncheckedCast(result.getProxy()).end_getAdapterName(result);
                notify();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }

        synchronized public String
        getResult()
        {
            while(_name == null)
            {
                try
                {
                    wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            return _name;
        }

        private String _name = null;
    }

    private static String
    getAdapterNameWithAMI(TestIntfPrx test)
    {
        GetAdapterNameCB cb = new GetAdapterNameCB();
        test.begin_getAdapterName(cb);
        return cb.getResult();
    }

    private static TestIntfPrx
    createTestIntfPrx(java.util.List<RemoteObjectAdapterPrx> adapters)
    {
        java.util.List<Ice.Endpoint> endpoints = new java.util.ArrayList<Ice.Endpoint>();
        TestIntfPrx test = null;
        for(RemoteObjectAdapterPrx p : adapters)
        {
            test = p.getTestIntf();
            Ice.Endpoint[] edpts = test.ice_getEndpoints();
            endpoints.addAll(java.util.Arrays.asList(edpts));
        }
        return TestIntfPrxHelper.uncheckedCast(
            test.ice_endpoints(endpoints.toArray(new Ice.Endpoint[endpoints.size()])));
    }

    private static void
    deactivate(RemoteCommunicatorPrx communicator, java.util.List<RemoteObjectAdapterPrx> adapters)
    {
        for(RemoteObjectAdapterPrx p : adapters)
        {
            communicator.deactivateObjectAdapter(p);
        }
    }

    public static void
    allTests(Application app)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String ref = "communicator:default -p 12010";
        RemoteCommunicatorPrx com = RemoteCommunicatorPrxHelper.uncheckedCast(communicator.stringToProxy(ref));

        out.print("testing binding with single endpoint... ");
        out.flush();
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
            catch(Ice.ConnectFailedException ex)
            {
                //
                // Usually the actual type of this exception is ConnectionRefusedException,
                // but not always. See bug 3179.
                //
            }
        }
        out.println("ok");

        out.print("testing binding with multiple endpoints... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("Adapter11", "default"));
            adapters.add(com.createObjectAdapter("Adapter12", "default"));
            adapters.add(com.createObjectAdapter("Adapter13", "default"));

            //
            // Ensure that when a connection is opened it's reused for new
            // proxies and that all endpoints are eventually tried.
            //
            java.util.Set<String> names = new java.util.HashSet<String>();
            names.add("Adapter11");
            names.add("Adapter12");
            names.add("Adapter13");
            while(!names.isEmpty())
            {
                java.util.List<RemoteObjectAdapterPrx> adpts =
                    new java.util.ArrayList<RemoteObjectAdapterPrx>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                java.util.Collections.shuffle(adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                java.util.Collections.shuffle(adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.remove(test1.getAdapterName());
                test1.ice_getConnection().close(false);
            }

            //
            // Ensure that the proxy correctly caches the connection (we
            // always send the request over the same connection.)
            //
            {
                for(RemoteObjectAdapterPrx p : adapters)
                {
                    p.getTestIntf().ice_ping();
                }

                TestIntfPrx test = createTestIntfPrx(adapters);
                String name = test.getAdapterName();
                final int nRetry = 10;
                int i;
                for(i = 0; i < nRetry && test.getAdapterName().equals(name); i++);
                test(i == nRetry);

                for(RemoteObjectAdapterPrx p : adapters)
                {
                    p.getTestIntf().ice_getConnection().close(false);
                }
            }

            //
            // Deactivate an adapter and ensure that we can still
            // establish the connection to the remaining adapters.
            //
            com.deactivateObjectAdapter(adapters.get(0));
            names.add("Adapter12");
            names.add("Adapter13");
            while(!names.isEmpty())
            {
                java.util.List<RemoteObjectAdapterPrx> adpts =
                    new java.util.ArrayList<RemoteObjectAdapterPrx>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                java.util.Collections.shuffle(adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                java.util.Collections.shuffle(adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.remove(test1.getAdapterName());
                test1.ice_getConnection().close(false);
            }

            //
            // Deactivate an adapter and ensure that we can still
            // establish the connection to the remaining adapter.
            //
            com.deactivateObjectAdapter(adapters.get(2));
            TestIntfPrx test = createTestIntfPrx(adapters);
            test(test.getAdapterName().equals("Adapter12"));

            deactivate(com, adapters);
        }
        out.println("ok");

        out.print("testing binding with multiple random endpoints... ");
        out.flush();
        {
            java.util.Random rand = new java.util.Random();

            RemoteObjectAdapterPrx[] adapters = new RemoteObjectAdapterPrx[5];
            adapters[0] = com.createObjectAdapter("AdapterRandom11", "default");
            adapters[1] = com.createObjectAdapter("AdapterRandom12", "default");
            adapters[2] = com.createObjectAdapter("AdapterRandom13", "default");
            adapters[3] = com.createObjectAdapter("AdapterRandom14", "default");
            adapters[4] = com.createObjectAdapter("AdapterRandom15", "default");

            boolean shortenTest =
                System.getProperty("os.name").startsWith("Windows") ||
                System.getProperty("java.vendor").toLowerCase().indexOf("android") >= 0;

            int count;
            if(shortenTest)
            {
                count = 60;
            }
            else
            {
                count = 20;
            }

            int adapterCount = adapters.length;
            while(--count > 0)
            {
                TestIntfPrx[] proxies;
                if(shortenTest)
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
                for(i = 0; i < proxies.length; ++i)
                {
                    RemoteObjectAdapterPrx[] adpts = new RemoteObjectAdapterPrx[rand.nextInt(adapters.length)];
                    if(adpts.length == 0)
                    {
                        adpts = new RemoteObjectAdapterPrx[1];
                    }
                    for(int j = 0; j < adpts.length; ++j)
                    {
                        adpts[j] = adapters[rand.nextInt(adapters.length)];
                    }
                    proxies[i] = createTestIntfPrx(java.util.Arrays.asList((adpts)));
                }

                for(TestIntfPrx p : proxies)
                {
                    p.begin_getAdapterName();
                }
                for(TestIntfPrx p : proxies)
                {
                    try
                    {
                        p.ice_ping();
                    }
                    catch(Ice.LocalException ex)
                    {
                    }
                }

                java.util.Set<Ice.Connection> connections = new java.util.HashSet<Ice.Connection>();
                for(TestIntfPrx p : proxies)
                {
                    if(p.ice_getCachedConnection() != null)
                    {
                        connections.add(p.ice_getCachedConnection());
                    }
                }
                test(connections.size() <= adapterCount);

                for(RemoteObjectAdapterPrx a : adapters)
                {
                    try
                    {
                        a.getTestIntf().ice_getConnection().close(false);
                    }
                    catch(Ice.LocalException ex)
                    {
                        // Expected if adapter is down.
                    }
                }
            }
        }
        out.println("ok");

        out.print("testing binding with multiple endpoints and AMI... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("AdapterAMI11", "default"));
            adapters.add(com.createObjectAdapter("AdapterAMI12", "default"));
            adapters.add(com.createObjectAdapter("AdapterAMI13", "default"));

            //
            // Ensure that when a connection is opened it's reused for new
            // proxies and that all endpoints are eventually tried.
            //
            java.util.Set<String> names = new java.util.HashSet<String>();
            names.add("AdapterAMI11");
            names.add("AdapterAMI12");
            names.add("AdapterAMI13");
            while(!names.isEmpty())
            {
                java.util.List<RemoteObjectAdapterPrx> adpts =
                    new java.util.ArrayList<RemoteObjectAdapterPrx>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                java.util.Collections.shuffle(adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                java.util.Collections.shuffle(adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.remove(getAdapterNameWithAMI(test1));
                test1.ice_getConnection().close(false);
            }

            //
            // Ensure that the proxy correctly caches the connection (we
            // always send the request over the same connection.)
            //
            {
                for(RemoteObjectAdapterPrx p : adapters)
                {
                    p.getTestIntf().ice_ping();
                }

                TestIntfPrx test = createTestIntfPrx(adapters);
                String name = getAdapterNameWithAMI(test);
                final int nRetry = 10;
                int i;
                for(i = 0; i < nRetry &&  getAdapterNameWithAMI(test).equals(name); i++);
                test(i == nRetry);

                for(RemoteObjectAdapterPrx p : adapters)
                {
                    p.getTestIntf().ice_getConnection().close(false);
                }
            }

            //
            // Deactivate an adapter and ensure that we can still
            // establish the connection to the remaining adapters.
            //
            com.deactivateObjectAdapter(adapters.get(0));
            names.add("AdapterAMI12");
            names.add("AdapterAMI13");
            while(!names.isEmpty())
            {
                java.util.List<RemoteObjectAdapterPrx> adpts =
                    new java.util.ArrayList<RemoteObjectAdapterPrx>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                java.util.Collections.shuffle(adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                java.util.Collections.shuffle(adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.remove(getAdapterNameWithAMI(test1));
                test1.ice_getConnection().close(false);
            }

            //
            // Deactivate an adapter and ensure that we can still
            // establish the connection to the remaining adapter.
            //
            com.deactivateObjectAdapter(adapters.get(2));
            TestIntfPrx test = createTestIntfPrx(adapters);
            test(getAdapterNameWithAMI(test).equals("AdapterAMI12"));

            deactivate(com, adapters);
        }
        out.println("ok");

        out.print("testing random endpoint selection... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("Adapter21", "default"));
            adapters.add(com.createObjectAdapter("Adapter22", "default"));
            adapters.add(com.createObjectAdapter("Adapter23", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

            java.util.Set<String> names = new java.util.HashSet<String>();
            names.add("Adapter21");
            names.add("Adapter22");
            names.add("Adapter23");
            while(!names.isEmpty())
            {
                names.remove(test.getAdapterName());
                test.ice_getConnection().close(false);
            }

            test = TestIntfPrxHelper.uncheckedCast(test.ice_endpointSelection(Ice.EndpointSelectionType.Random));
            test(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

            names.add("Adapter21");
            names.add("Adapter22");
            names.add("Adapter23");
            while(!names.isEmpty())
            {
                names.remove(test.getAdapterName());
                test.ice_getConnection().close(false);
            }

            deactivate(com, adapters);
        }
        out.println("ok");

        out.print("testing ordered endpoint selection... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("Adapter31", "default"));
            adapters.add(com.createObjectAdapter("Adapter32", "default"));
            adapters.add(com.createObjectAdapter("Adapter33", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test = TestIntfPrxHelper.uncheckedCast(test.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
            test(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            int nRetry = 5;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter31"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(0));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter32"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(1));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter33"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(2));

            try
            {
                test.getAdapterName();
            }
            catch(Ice.ConnectFailedException ex)
            {
                //
                // Usually the actual type of this exception is ConnectionRefusedException,
                // but not always. See bug 3179.
                //
            }

            Ice.Endpoint[] endpoints = test.ice_getEndpoints();

            adapters.clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.add(com.createObjectAdapter("Adapter36", endpoints[2].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter36"); i++);
            test(i == nRetry);
            test.ice_getConnection().close(false);
            adapters.add(com.createObjectAdapter("Adapter35", endpoints[1].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter35"); i++);
            test(i == nRetry);
            test.ice_getConnection().close(false);
            adapters.add(com.createObjectAdapter("Adapter34", endpoints[0].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter34"); i++);
            test(i == nRetry);

            deactivate(com, adapters);
        }
        out.println("ok");

        out.print("testing per request binding with single endpoint... ");
        out.flush();
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
            catch(Ice.ConnectFailedException ex)
            {
                //
                // Usually the actual type of this exception is ConnectionRefusedException,
                // but not always. See bug 3179.
                //
            }
        }
        out.println("ok");

        out.print("testing per request binding with multiple endpoints... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("Adapter51", "default"));
            adapters.add(com.createObjectAdapter("Adapter52", "default"));
            adapters.add(com.createObjectAdapter("Adapter53", "default"));

            TestIntfPrx test = TestIntfPrxHelper.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(false));
            test(!test.ice_isConnectionCached());

            java.util.Set<String> names = new java.util.HashSet<String>();
            names.add("Adapter51");
            names.add("Adapter52");
            names.add("Adapter53");
            while(!names.isEmpty())
            {
                names.remove(test.getAdapterName());
            }

            com.deactivateObjectAdapter(adapters.get(0));

            names.add("Adapter52");
            names.add("Adapter53");
            while(!names.isEmpty())
            {
                names.remove(test.getAdapterName());
            }

            com.deactivateObjectAdapter(adapters.get(2));

            test(test.getAdapterName().equals("Adapter52"));

            deactivate(com, adapters);
        }
        out.println("ok");

        out.print("testing per request binding with multiple endpoints and AMI... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("AdapterAMI51", "default"));
            adapters.add(com.createObjectAdapter("AdapterAMI52", "default"));
            adapters.add(com.createObjectAdapter("AdapterAMI53", "default"));

            TestIntfPrx test = TestIntfPrxHelper.uncheckedCast(createTestIntfPrx(adapters).ice_connectionCached(false));
            test(!test.ice_isConnectionCached());

            java.util.Set<String> names = new java.util.HashSet<String>();
            names.add("AdapterAMI51");
            names.add("AdapterAMI52");
            names.add("AdapterAMI53");
            while(!names.isEmpty())
            {
                names.remove(getAdapterNameWithAMI(test));
            }

            com.deactivateObjectAdapter(adapters.get(0));

            names.add("AdapterAMI52");
            names.add("AdapterAMI53");
            while(!names.isEmpty())
            {
                names.remove(getAdapterNameWithAMI(test));
            }

            com.deactivateObjectAdapter(adapters.get(2));

            test(getAdapterNameWithAMI(test).equals("AdapterAMI52"));

            deactivate(com, adapters);
        }
        out.println("ok");

        out.print("testing per request binding and ordered endpoint selection... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("Adapter61", "default"));
            adapters.add(com.createObjectAdapter("Adapter62", "default"));
            adapters.add(com.createObjectAdapter("Adapter63", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test = TestIntfPrxHelper.uncheckedCast(test.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
            test(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            test = TestIntfPrxHelper.uncheckedCast(test.ice_connectionCached(false));
            test(!test.ice_isConnectionCached());
            int nRetry = 5;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter61"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(0));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter62"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(1));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter63"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(2));

            try
            {
                test.getAdapterName();
            }
            catch(Ice.ConnectFailedException ex)
            {
                //
                // Usually the actual type of this exception is ConnectionRefusedException,
                // but not always. See bug 3179.
                //
            }

            Ice.Endpoint[] endpoints = test.ice_getEndpoints();

            adapters.clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.add(com.createObjectAdapter("Adapter66", endpoints[2].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter66"); i++);
            test(i == nRetry);
            adapters.add(com.createObjectAdapter("Adapter65", endpoints[1].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter65"); i++);
            test(i == nRetry);
            adapters.add(com.createObjectAdapter("Adapter64", endpoints[0].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter64"); i++);
            test(i == nRetry);

            deactivate(com, adapters);
        }
        out.println("ok");

        out.print("testing per request binding and ordered endpoint selection and AMI... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("AdapterAMI61", "default"));
            adapters.add(com.createObjectAdapter("AdapterAMI62", "default"));
            adapters.add(com.createObjectAdapter("AdapterAMI63", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test = TestIntfPrxHelper.uncheckedCast(test.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
            test(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            test = TestIntfPrxHelper.uncheckedCast(test.ice_connectionCached(false));
            test(!test.ice_isConnectionCached());
            int nRetry = 5;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI61"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(0));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI62"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(1));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI63"); i++);
            test(i == nRetry);
            com.deactivateObjectAdapter(adapters.get(2));

            try
            {
                test.getAdapterName();
            }
            catch(Ice.ConnectFailedException ex)
            {
                //
                // Usually the actual type of this exception is ConnectionRefusedException,
                // but not always. See bug 3179.
                //
            }

            Ice.Endpoint[] endpoints = test.ice_getEndpoints();

            adapters.clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.add(com.createObjectAdapter("AdapterAMI66", endpoints[2].toString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI66"); i++);
            test(i == nRetry);
            adapters.add(com.createObjectAdapter("AdapterAMI65", endpoints[1].toString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI65"); i++);
            test(i == nRetry);
            adapters.add(com.createObjectAdapter("AdapterAMI64", endpoints[0].toString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI64"); i++);
            test(i == nRetry);

            deactivate(com, adapters);
        }
        out.println("ok");

        out.print("testing endpoint mode filtering... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
            adapters.add(com.createObjectAdapter("Adapter71", "default"));
            adapters.add(com.createObjectAdapter("Adapter72", "udp"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test(test.getAdapterName().equals("Adapter71"));

            TestIntfPrx testUDP = TestIntfPrxHelper.uncheckedCast(test.ice_datagram());
            test(test.ice_getConnection() != testUDP.ice_getConnection());
            try
            {
                testUDP.getAdapterName();
            }
            catch(Ice.TwowayOnlyException ex)
            {
            }
        }
        out.println("ok");

        if(communicator.getProperties().getProperty("Ice.Plugin.IceSSL").length() > 0)
        {
            out.print("testing unsecure vs. secure endpoints... ");
            out.flush();
            {
                java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<RemoteObjectAdapterPrx>();
                adapters.add(com.createObjectAdapter("Adapter81", "ssl"));
                adapters.add(com.createObjectAdapter("Adapter82", "tcp"));

                TestIntfPrx test = createTestIntfPrx(adapters);
                int i;
                for(i = 0; i < 5; i++)
                {
                    test(test.getAdapterName().equals("Adapter82"));
                    test.ice_getConnection().close(false);
                }

                TestIntfPrx testSecure = TestIntfPrxHelper.uncheckedCast(test.ice_secure(true));
                test(testSecure.ice_isSecure());
                testSecure = TestIntfPrxHelper.uncheckedCast(test.ice_secure(false));
                test(!testSecure.ice_isSecure());
                testSecure = TestIntfPrxHelper.uncheckedCast(test.ice_secure(true));
                test(testSecure.ice_isSecure());
                test(test.ice_getConnection() != testSecure.ice_getConnection());

                com.deactivateObjectAdapter(adapters.get(1));

                for(i = 0; i < 5; i++)
                {
                    test(test.getAdapterName().equals("Adapter81"));
                    test.ice_getConnection().close(false);
                }

                com.createObjectAdapter("Adapter83", (test.ice_getEndpoints()[1]).toString()); // Reactive tcp OA.

                for(i = 0; i < 5; i++)
                {
                    test(test.getAdapterName().equals("Adapter83"));
                    test.ice_getConnection().close(false);
                }

                com.deactivateObjectAdapter(adapters.get(0));
                try
                {
                    testSecure.ice_ping();
                    test(false);
                }
                catch(Ice.ConnectFailedException ex)
                {
                    //
                    // Usually the actual type of this exception is ConnectionRefusedException,
                    // but not always. See bug 3179.
                    //
                }

                deactivate(com, adapters);
            }
            out.println("ok");
        }

        {
            out.print("testing ipv4 & ipv6 connections... ");
            out.flush();

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

            java.util.List<Ice.Properties> clientProps = new java.util.ArrayList<Ice.Properties>();
            clientProps.add(ipv4);
            clientProps.add(ipv6);
            clientProps.add(bothPreferIPv4);
            clientProps.add(bothPreferIPv6);

            Ice.Properties anyipv4 = ipv4._clone();
            anyipv4.setProperty("Adapter.Endpoints", "tcp -p 12012");
            anyipv4.setProperty("Adapter.PublishedEndpoints", "tcp -h 127.0.0.1 -p 12012");

            Ice.Properties anyipv6 = ipv6._clone();
            anyipv6.setProperty("Adapter.Endpoints", "tcp -p 12012");
            anyipv6.setProperty("Adapter.PublishedEndpoints", "tcp -h \".1\" -p 12012");

            Ice.Properties anyboth = Ice.Util.createProperties();
            anyboth.setProperty("Ice.IPv4", "1");
            anyboth.setProperty("Ice.IPv6", "1");
            anyboth.setProperty("Adapter.Endpoints", "tcp -p 12012");
            anyboth.setProperty("Adapter.PublishedEndpoints", "tcp -h \"::1\" -p 12012:tcp -h 127.0.0.1 -p 12012");

            Ice.Properties localipv4 = ipv4._clone();
            localipv4.setProperty("Adapter.Endpoints", "tcp -h 127.0.0.1");

            Ice.Properties localipv6 = ipv6._clone();
            localipv6.setProperty("Adapter.Endpoints", "tcp -h \"::1\"");

            java.util.List<Ice.Properties> serverProps = new java.util.ArrayList<Ice.Properties>(clientProps);
            serverProps.add(anyipv4);
            serverProps.add(anyipv6);
            serverProps.add(anyboth);
            serverProps.add(localipv4);
            serverProps.add(localipv6);

            boolean ipv6NotSupported = false;
            for(Ice.Properties p : serverProps)
            {
                Ice.InitializationData serverInitData = app.createInitializationData();
                serverInitData.properties = p;
                Ice.Communicator serverCommunicator = app.initialize(serverInitData);
                Ice.ObjectAdapter oa;
                try
                {
                    oa = serverCommunicator.createObjectAdapter("Adapter");
                    oa.activate();
                }
                catch(Ice.DNSException ex)
                {
                    serverCommunicator.destroy();
                    continue; // IP version not supported.
                }
                catch(Ice.SocketException ex)
                {
                    if(p == ipv6)
                    {
                        ipv6NotSupported = true;
                    }
                    serverCommunicator.destroy();
                    continue; // IP version not supported.
                }

                String strPrx = oa.createProxy(serverCommunicator.stringToIdentity("dummy")).toString();
                for(Ice.Properties q : clientProps)
                {
                    Ice.InitializationData clientInitData = app.createInitializationData();
                    clientInitData.properties = q;
                    Ice.Communicator clientCommunicator = app.initialize(clientInitData);
                    Ice.ObjectPrx prx = clientCommunicator.stringToProxy(strPrx);
                    try
                    {
                        prx.ice_ping();
                        test(false);
                    }
                    catch(Ice.ObjectNotExistException ex)
                    {
                        // Expected, no object registered.
                    }
                    catch(Ice.DNSException ex)
                    {
                        // Expected if no IPv4 or IPv6 address is
                        // associated to localhost or if trying to connect
                        // to an any endpoint with the wrong IP version,
                        // e.g.: resolving an IPv4 address when only IPv6
                        // is enabled fails with a DNS exception.
                    }
                    catch(Ice.SocketException ex)
                    {
                        test((p == ipv4 && q == ipv6) || (p == ipv6 && q == ipv4) ||
                             (p == bothPreferIPv4 && q == ipv6) || (p == bothPreferIPv6 && q == ipv4) ||
                             (p == bothPreferIPv6 && q == ipv6 && ipv6NotSupported) ||
                             (p == anyipv4 && q == ipv6) || (p == anyipv6 && q == ipv4) ||
                             (p == localipv4 && q == ipv6) || (p == localipv6 && q == ipv4) ||
                             (p == ipv6 && q == bothPreferIPv4) || (p == bothPreferIPv6 && q == ipv6) ||
                             (p == ipv6 && q == bothPreferIPv4) || (p == ipv6 && q == bothPreferIPv6) || 
                             (p == bothPreferIPv6 && q == ipv6));
                    }
                    clientCommunicator.destroy();
                }
                serverCommunicator.destroy();
            }
            
            out.println("ok");
        }

        com.shutdown();
    }
}
