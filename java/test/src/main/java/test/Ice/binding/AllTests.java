// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;

import java.io.PrintWriter;

import test.Ice.binding.Test.RemoteCommunicatorPrx;
import test.Ice.binding.Test.RemoteObjectAdapterPrx;
import test.Ice.binding.Test.TestIntfPrx;
import test.Util.Application;

import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.EndpointSelectionType;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static String getAdapterNameWithAMI(TestIntfPrx test)
    {
        return test.getAdapterNameAsync().join();
    }

    private static TestIntfPrx createTestIntfPrx(java.util.List<RemoteObjectAdapterPrx> adapters)
    {
        java.util.List<Endpoint> endpoints = new java.util.ArrayList<>();
        TestIntfPrx test = null;
        for(RemoteObjectAdapterPrx p : adapters)
        {
            test = p.getTestIntf();
            Endpoint[] edpts = test.ice_getEndpoints();
            endpoints.addAll(java.util.Arrays.asList(edpts));
        }
        return test.ice_endpoints(endpoints.toArray(new Endpoint[endpoints.size()]));
    }

    private static void deactivate(RemoteCommunicatorPrx communicator, java.util.List<RemoteObjectAdapterPrx> adapters)
    {
        for(RemoteObjectAdapterPrx p : adapters)
        {
            communicator.deactivateObjectAdapter(p);
        }
    }

    public static void allTests(Application app)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String ref = "communicator:" + app.getTestEndpoint(0);
        RemoteCommunicatorPrx rcom = RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

        out.print("testing binding with single endpoint... ");
        out.flush();
        {
            RemoteObjectAdapterPrx adapter = rcom.createObjectAdapter("Adapter", "default");

            TestIntfPrx test1 = adapter.getTestIntf();
            TestIntfPrx test2 = adapter.getTestIntf();
            test(test1.ice_getConnection() == test2.ice_getConnection());

            test1.ice_ping();
            test2.ice_ping();

            rcom.deactivateObjectAdapter(adapter);

            TestIntfPrx test3 = TestIntfPrx.uncheckedCast(test1);
            test(test3.ice_getConnection() == test1.ice_getConnection());
            test(test3.ice_getConnection() == test2.ice_getConnection());

            try
            {
                test3.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectFailedException ex)
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
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter11", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter12", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter13", "default"));

            //
            // Ensure that when a connection is opened it's reused for new
            // proxies and that all endpoints are eventually tried.
            //
            java.util.Set<String> names = new java.util.HashSet<>();
            names.add("Adapter11");
            names.add("Adapter12");
            names.add("Adapter13");
            while(!names.isEmpty())
            {
                java.util.List<RemoteObjectAdapterPrx> adpts = new java.util.ArrayList<>(adapters);

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
            rcom.deactivateObjectAdapter(adapters.get(0));
            names.add("Adapter12");
            names.add("Adapter13");
            while(!names.isEmpty())
            {
                java.util.List<RemoteObjectAdapterPrx> adpts = new java.util.ArrayList<>(adapters);

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
            rcom.deactivateObjectAdapter(adapters.get(2));
            TestIntfPrx test = createTestIntfPrx(adapters);
            test(test.getAdapterName().equals("Adapter12"));

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing binding with multiple random endpoints... ");
        out.flush();
        {
            java.util.Random rand = new java.util.Random();

            RemoteObjectAdapterPrx[] adapters = new RemoteObjectAdapterPrx[5];
            adapters[0] = rcom.createObjectAdapter("AdapterRandom11", "default");
            adapters[1] = rcom.createObjectAdapter("AdapterRandom12", "default");
            adapters[2] = rcom.createObjectAdapter("AdapterRandom13", "default");
            adapters[3] = rcom.createObjectAdapter("AdapterRandom14", "default");
            adapters[4] = rcom.createObjectAdapter("AdapterRandom15", "default");

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
                        rcom.deactivateObjectAdapter(adapters[4]);
                        --adapterCount;
                    }
                    proxies = new TestIntfPrx[10];
                }
                else
                {
                    if(count < 60 && count % 10 == 0)
                    {
                        rcom.deactivateObjectAdapter(adapters[count / 10 - 1]);
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
                    p.getAdapterNameAsync();
                }
                for(TestIntfPrx p : proxies)
                {
                    try
                    {
                        p.ice_ping();
                    }
                    catch(com.zeroc.Ice.LocalException ex)
                    {
                    }
                }

                java.util.Set<com.zeroc.Ice.Connection> connections = new java.util.HashSet<>();
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
                    catch(com.zeroc.Ice.LocalException ex)
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
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("AdapterAMI11", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI12", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI13", "default"));

            //
            // Ensure that when a connection is opened it's reused for new
            // proxies and that all endpoints are eventually tried.
            //
            java.util.Set<String> names = new java.util.HashSet<>();
            names.add("AdapterAMI11");
            names.add("AdapterAMI12");
            names.add("AdapterAMI13");
            while(!names.isEmpty())
            {
                java.util.List<RemoteObjectAdapterPrx> adpts = new java.util.ArrayList<>(adapters);

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
            rcom.deactivateObjectAdapter(adapters.get(0));
            names.add("AdapterAMI12");
            names.add("AdapterAMI13");
            while(!names.isEmpty())
            {
                java.util.List<RemoteObjectAdapterPrx> adpts = new java.util.ArrayList<>(adapters);

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
            rcom.deactivateObjectAdapter(adapters.get(2));
            TestIntfPrx test = createTestIntfPrx(adapters);
            test(getAdapterNameWithAMI(test).equals("AdapterAMI12"));

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing random endpoint selection... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter21", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter22", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter23", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test(test.ice_getEndpointSelection() == EndpointSelectionType.Random);

            java.util.Set<String> names = new java.util.HashSet<>();
            names.add("Adapter21");
            names.add("Adapter22");
            names.add("Adapter23");
            while(!names.isEmpty())
            {
                names.remove(test.getAdapterName());
                test.ice_getConnection().close(false);
            }

            test = test.ice_endpointSelection(EndpointSelectionType.Random);
            test(test.ice_getEndpointSelection() == EndpointSelectionType.Random);

            names.add("Adapter21");
            names.add("Adapter22");
            names.add("Adapter23");
            while(!names.isEmpty())
            {
                names.remove(test.getAdapterName());
                test.ice_getConnection().close(false);
            }

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing ordered endpoint selection... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter31", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter32", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter33", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test = test.ice_endpointSelection(EndpointSelectionType.Ordered);
            test(test.ice_getEndpointSelection() == EndpointSelectionType.Ordered);
            int nRetry = 5;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter31"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(0));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter32"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(1));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter33"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(2));

            try
            {
                test.getAdapterName();
            }
            catch(com.zeroc.Ice.ConnectFailedException ex)
            {
                //
                // Usually the actual type of this exception is ConnectionRefusedException,
                // but not always. See bug 3179.
                //
            }

            Endpoint[] endpoints = test.ice_getEndpoints();

            adapters.clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.add(rcom.createObjectAdapter("Adapter36", endpoints[2].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter36"); i++);
            test(i == nRetry);
            test.ice_getConnection().close(false);
            adapters.add(rcom.createObjectAdapter("Adapter35", endpoints[1].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter35"); i++);
            test(i == nRetry);
            test.ice_getConnection().close(false);
            adapters.add(rcom.createObjectAdapter("Adapter34", endpoints[0].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter34"); i++);
            test(i == nRetry);

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing per request binding with single endpoint... ");
        out.flush();
        {
            RemoteObjectAdapterPrx adapter = rcom.createObjectAdapter("Adapter41", "default");

            TestIntfPrx test1 = adapter.getTestIntf().ice_connectionCached(false);
            TestIntfPrx test2 = adapter.getTestIntf().ice_connectionCached(false);
            test(!test1.ice_isConnectionCached());
            test(!test2.ice_isConnectionCached());
            test(test1.ice_getConnection() != null && test2.ice_getConnection() != null);
            test(test1.ice_getConnection() == test2.ice_getConnection());

            test1.ice_ping();

            rcom.deactivateObjectAdapter(adapter);

            TestIntfPrx test3 = TestIntfPrx.uncheckedCast(test1);
            try
            {
                test(test3.ice_getConnection() == test1.ice_getConnection());
                test(false);
            }
            catch(com.zeroc.Ice.ConnectFailedException ex)
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
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter51", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter52", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter53", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters).ice_connectionCached(false);
            test(!test.ice_isConnectionCached());

            java.util.Set<String> names = new java.util.HashSet<>();
            names.add("Adapter51");
            names.add("Adapter52");
            names.add("Adapter53");
            while(!names.isEmpty())
            {
                names.remove(test.getAdapterName());
            }

            rcom.deactivateObjectAdapter(adapters.get(0));

            names.add("Adapter52");
            names.add("Adapter53");
            while(!names.isEmpty())
            {
                names.remove(test.getAdapterName());
            }

            rcom.deactivateObjectAdapter(adapters.get(2));

            test(test.getAdapterName().equals("Adapter52"));

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing per request binding with multiple endpoints and AMI... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("AdapterAMI51", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI52", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI53", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters).ice_connectionCached(false);
            test(!test.ice_isConnectionCached());

            java.util.Set<String> names = new java.util.HashSet<>();
            names.add("AdapterAMI51");
            names.add("AdapterAMI52");
            names.add("AdapterAMI53");
            while(!names.isEmpty())
            {
                names.remove(getAdapterNameWithAMI(test));
            }

            rcom.deactivateObjectAdapter(adapters.get(0));

            names.add("AdapterAMI52");
            names.add("AdapterAMI53");
            while(!names.isEmpty())
            {
                names.remove(getAdapterNameWithAMI(test));
            }

            rcom.deactivateObjectAdapter(adapters.get(2));

            test(getAdapterNameWithAMI(test).equals("AdapterAMI52"));

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing per request binding and ordered endpoint selection... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter61", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter62", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter63", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test = test.ice_endpointSelection(EndpointSelectionType.Ordered);
            test(test.ice_getEndpointSelection() == EndpointSelectionType.Ordered);
            test = test.ice_connectionCached(false);
            test(!test.ice_isConnectionCached());
            int nRetry = 5;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter61"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(0));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter62"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(1));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter63"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(2));

            try
            {
                test.getAdapterName();
            }
            catch(com.zeroc.Ice.ConnectFailedException ex)
            {
                //
                // Usually the actual type of this exception is ConnectionRefusedException,
                // but not always. See bug 3179.
                //
            }

            Endpoint[] endpoints = test.ice_getEndpoints();

            adapters.clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.add(rcom.createObjectAdapter("Adapter66", endpoints[2].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter66"); i++);
            test(i == nRetry);
            adapters.add(rcom.createObjectAdapter("Adapter65", endpoints[1].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter65"); i++);
            test(i == nRetry);
            adapters.add(rcom.createObjectAdapter("Adapter64", endpoints[0].toString()));
            for(i = 0; i < nRetry && test.getAdapterName().equals("Adapter64"); i++);
            test(i == nRetry);

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing per request binding and ordered endpoint selection and AMI... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("AdapterAMI61", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI62", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI63", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test = test.ice_endpointSelection(EndpointSelectionType.Ordered);
            test(test.ice_getEndpointSelection() == EndpointSelectionType.Ordered);
            test = test.ice_connectionCached(false);
            test(!test.ice_isConnectionCached());
            int nRetry = 5;
            int i;

            //
            // Ensure that endpoints are tried in order by deactiving the adapters
            // one after the other.
            //
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI61"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(0));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI62"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(1));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI63"); i++);
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(2));

            try
            {
                test.getAdapterName();
            }
            catch(com.zeroc.Ice.ConnectFailedException ex)
            {
                //
                // Usually the actual type of this exception is ConnectionRefusedException,
                // but not always. See bug 3179.
                //
            }

            Endpoint[] endpoints = test.ice_getEndpoints();

            adapters.clear();

            //
            // Now, re-activate the adapters with the same endpoints in the opposite
            // order.
            //
            adapters.add(rcom.createObjectAdapter("AdapterAMI66", endpoints[2].toString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI66"); i++);
            test(i == nRetry);
            adapters.add(rcom.createObjectAdapter("AdapterAMI65", endpoints[1].toString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI65"); i++);
            test(i == nRetry);
            adapters.add(rcom.createObjectAdapter("AdapterAMI64", endpoints[0].toString()));
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test).equals("AdapterAMI64"); i++);
            test(i == nRetry);

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing endpoint mode filtering... ");
        out.flush();
        {
            java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter71", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter72", "udp"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test(test.getAdapterName().equals("Adapter71"));

            TestIntfPrx testUDP = test.ice_datagram();
            test(test.ice_getConnection() != testUDP.ice_getConnection());
            try
            {
                testUDP.getAdapterName();
            }
            catch(java.lang.IllegalArgumentException ex)
            {
            }
        }
        out.println("ok");

        if(communicator.getProperties().getProperty("Ice.Plugin.IceSSL").length() > 0)
        {
            out.print("testing unsecure vs. secure endpoints... ");
            out.flush();
            {
                java.util.List<RemoteObjectAdapterPrx> adapters = new java.util.ArrayList<>();
                adapters.add(rcom.createObjectAdapter("Adapter81", "ssl"));
                adapters.add(rcom.createObjectAdapter("Adapter82", "tcp"));

                TestIntfPrx test = createTestIntfPrx(adapters);
                int i;
                for(i = 0; i < 5; i++)
                {
                    test(test.getAdapterName().equals("Adapter82"));
                    test.ice_getConnection().close(false);
                }

                TestIntfPrx testSecure = test.ice_secure(true);
                test(testSecure.ice_isSecure());
                testSecure = test.ice_secure(false);
                test(!testSecure.ice_isSecure());
                testSecure = test.ice_secure(true);
                test(testSecure.ice_isSecure());
                test(test.ice_getConnection() != testSecure.ice_getConnection());

                rcom.deactivateObjectAdapter(adapters.get(1));

                for(i = 0; i < 5; i++)
                {
                    test(test.getAdapterName().equals("Adapter81"));
                    test.ice_getConnection().close(false);
                }

                rcom.createObjectAdapter("Adapter83", (test.ice_getEndpoints()[1]).toString()); // Reactive tcp OA.

                for(i = 0; i < 5; i++)
                {
                    test(test.getAdapterName().equals("Adapter83"));
                    test.ice_getConnection().close(false);
                }

                rcom.deactivateObjectAdapter(adapters.get(0));
                try
                {
                    testSecure.ice_ping();
                    test(false);
                }
                catch(com.zeroc.Ice.ConnectFailedException ex)
                {
                    //
                    // Usually the actual type of this exception is ConnectionRefusedException,
                    // but not always. See bug 3179.
                    //
                }

                deactivate(rcom, adapters);
            }
            out.println("ok");
        }

        {
            out.print("testing ipv4 & ipv6 connections... ");
            out.flush();

            com.zeroc.Ice.Properties ipv4 = com.zeroc.Ice.Util.createProperties();
            ipv4.setProperty("Ice.IPv4", "1");
            ipv4.setProperty("Ice.IPv6", "0");
            ipv4.setProperty("Adapter.Endpoints", "tcp -h localhost");

            com.zeroc.Ice.Properties ipv6 = com.zeroc.Ice.Util.createProperties();
            ipv6.setProperty("Ice.IPv4", "0");
            ipv6.setProperty("Ice.IPv6", "1");
            ipv6.setProperty("Adapter.Endpoints", "tcp -h localhost");

            com.zeroc.Ice.Properties bothPreferIPv4 = com.zeroc.Ice.Util.createProperties();
            bothPreferIPv4.setProperty("Ice.IPv4", "1");
            bothPreferIPv4.setProperty("Ice.IPv6", "1");
            bothPreferIPv4.setProperty("Ice.PreferIPv6Address", "0");
            bothPreferIPv4.setProperty("Adapter.Endpoints", "tcp -h localhost");

            com.zeroc.Ice.Properties bothPreferIPv6 = com.zeroc.Ice.Util.createProperties();
            bothPreferIPv6.setProperty("Ice.IPv4", "1");
            bothPreferIPv6.setProperty("Ice.IPv6", "1");
            bothPreferIPv6.setProperty("Ice.PreferIPv6Address", "1");
            bothPreferIPv6.setProperty("Adapter.Endpoints", "tcp -h localhost");

            java.util.List<com.zeroc.Ice.Properties> clientProps = new java.util.ArrayList<>();
            clientProps.add(ipv4);
            clientProps.add(ipv6);
            clientProps.add(bothPreferIPv4);
            clientProps.add(bothPreferIPv6);

            String endpoint = "tcp -p " + app.getTestPort(2);

            com.zeroc.Ice.Properties anyipv4 = ipv4._clone();
            anyipv4.setProperty("Adapter.Endpoints", endpoint);
            anyipv4.setProperty("Adapter.PublishedEndpoints", endpoint + " -h 127.0.0.1");

            com.zeroc.Ice.Properties anyipv6 = ipv6._clone();
            anyipv6.setProperty("Adapter.Endpoints", endpoint);
            anyipv6.setProperty("Adapter.PublishedEndpoints", endpoint + " -h \".1\"");

            com.zeroc.Ice.Properties anyboth = com.zeroc.Ice.Util.createProperties();
            anyboth.setProperty("Ice.IPv4", "1");
            anyboth.setProperty("Ice.IPv6", "1");
            anyboth.setProperty("Adapter.Endpoints", endpoint);
            anyboth.setProperty("Adapter.PublishedEndpoints", endpoint + " -h \"::1\":" + endpoint + " -h 127.0.0.1");

            com.zeroc.Ice.Properties localipv4 = ipv4._clone();
            localipv4.setProperty("Adapter.Endpoints", "tcp -h 127.0.0.1");

            com.zeroc.Ice.Properties localipv6 = ipv6._clone();
            localipv6.setProperty("Adapter.Endpoints", "tcp -h \"::1\"");

            java.util.List<com.zeroc.Ice.Properties> serverProps = new java.util.ArrayList<>(clientProps);
            serverProps.add(anyipv4);
            serverProps.add(anyipv6);
            serverProps.add(anyboth);
            serverProps.add(localipv4);
            serverProps.add(localipv6);

            boolean ipv6NotSupported = false;
            for(com.zeroc.Ice.Properties p : serverProps)
            {
                com.zeroc.Ice.InitializationData serverInitData = app.createInitializationData();
                serverInitData.properties = p;
                com.zeroc.Ice.Communicator serverCommunicator = app.initialize(serverInitData);
                com.zeroc.Ice.ObjectAdapter oa;
                try
                {
                    oa = serverCommunicator.createObjectAdapter("Adapter");
                    oa.activate();
                }
                catch(com.zeroc.Ice.DNSException ex)
                {
                    serverCommunicator.destroy();
                    continue; // IP version not supported.
                }
                catch(com.zeroc.Ice.SocketException ex)
                {
                    if(p == ipv6)
                    {
                        ipv6NotSupported = true;
                    }
                    serverCommunicator.destroy();
                    continue; // IP version not supported.
                }

                String strPrx = oa.createProxy(com.zeroc.Ice.Util.stringToIdentity("dummy")).toString();
                for(com.zeroc.Ice.Properties q : clientProps)
                {
                    com.zeroc.Ice.InitializationData clientInitData = app.createInitializationData();
                    clientInitData.properties = q;
                    com.zeroc.Ice.Communicator clientCommunicator = app.initialize(clientInitData);
                    com.zeroc.Ice.ObjectPrx prx = clientCommunicator.stringToProxy(strPrx);
                    try
                    {
                        prx.ice_ping();
                        test(false);
                    }
                    catch(com.zeroc.Ice.ObjectNotExistException ex)
                    {
                        // Expected, no object registered.
                    }
                    catch(com.zeroc.Ice.DNSException ex)
                    {
                        // Expected if no IPv4 or IPv6 address is
                        // associated to localhost or if trying to connect
                        // to an any endpoint with the wrong IP version,
                        // e.g.: resolving an IPv4 address when only IPv6
                        // is enabled fails with a DNS exception.
                    }
                    catch(com.zeroc.Ice.SocketException ex)
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

        //
        // On Windows, the FD limit is very high and there's no way to limit the number of FDs
        // for the server so we don't run this test.
        //
        if(!System.getProperty("os.name").startsWith("Windows"))
        {
            out.print("testing FD limit... ");
            out.flush();

            RemoteObjectAdapterPrx adapter = rcom.createObjectAdapter("Adapter", "default");

            TestIntfPrx test = adapter.getTestIntf();
            int i = 0;
            while(true)
            {
                try
                {
                    test.ice_connectionId(Integer.toString(i)).ice_ping();
                    ++i;
                }
                catch(com.zeroc.Ice.LocalException ex)
                {
                    break;
                }
            }

            try
            {
                test.ice_connectionId(Integer.toString(i)).ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectionRefusedException ex)
            {
                // Close the connection now to free a FD (it could be done after the sleep but
                // there could be race condiutation since the connection might not be closed
                // immediately due to threading).
                test.ice_connectionId("0").ice_getConnection().close(false);

                //
                // The server closed the acceptor, wait one second and retry after freeing a FD.
                //
                try
                {
                    Thread.sleep(1100);
                }
                catch(InterruptedException ex1)
                {
                }

                try
                {
                    test.ice_connectionId(Integer.toString(i)).ice_ping();
                }
                catch(com.zeroc.Ice.LocalException ex1)
                {
                    test(false);
                }
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                // The server didn't close the acceptor but we still get a failure (it's possible
                // that the client reached the FD limit depending on the server we are running
                // against...).
            }

            out.println("ok");
        }

        rcom.shutdown();
    }
}
