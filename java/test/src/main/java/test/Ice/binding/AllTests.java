// Copyright (c) ZeroC, Inc.

package test.Ice.binding;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectFailedException;
import com.zeroc.Ice.Connection;
import com.zeroc.Ice.DNSException;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.EndpointSelectionType;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.TwowayOnlyException;
import com.zeroc.Ice.Util;

import test.Ice.binding.Test.RemoteCommunicatorPrx;
import test.Ice.binding.Test.RemoteObjectAdapterPrx;
import test.Ice.binding.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static String getAdapterNameWithAMI(TestIntfPrx test) {
        return test.getAdapterNameAsync().join();
    }

    private static TestIntfPrx createTestIntfPrx(List<RemoteObjectAdapterPrx> adapters) {
        List<Endpoint> endpoints = new ArrayList<>();
        TestIntfPrx test = null;
        for (RemoteObjectAdapterPrx p : adapters) {
            test = p.getTestIntf();
            Endpoint[] edpts = test.ice_getEndpoints();
            endpoints.addAll(Arrays.asList(edpts));
        }
        return test.ice_endpoints(endpoints.toArray(new Endpoint[endpoints.size()]));
    }

    private static void deactivate(
            RemoteCommunicatorPrx communicator, List<RemoteObjectAdapterPrx> adapters) {
        for (RemoteObjectAdapterPrx p : adapters) {
            communicator.deactivateObjectAdapter(p);
        }
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String ref = "communicator:" + helper.getTestEndpoint(0);
        var rcom = RemoteCommunicatorPrx.createProxy(communicator, ref);

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

            try {
                test3.ice_ping();
                test(false);
            } catch (ConnectFailedException ex) {
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
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter11", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter12", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter13", "default"));

            //
            // Ensure that when a connection is opened it's reused for new
            // proxies and that all endpoints are eventually tried.
            //
            Set<String> names = new HashSet<>();
            names.add("Adapter11");
            names.add("Adapter12");
            names.add("Adapter13");
            while (!names.isEmpty()) {
                List<RemoteObjectAdapterPrx> adpts = new ArrayList<>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                Collections.shuffle(adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                Collections.shuffle(adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.remove(test1.getAdapterName());
                test1.ice_getConnection().close();
            }

            //
            // Ensure that the proxy correctly caches the connection (we always send the request
            // over the same connection.)
            //
            {
                for (RemoteObjectAdapterPrx p : adapters) {
                    p.getTestIntf().ice_ping();
                }

                TestIntfPrx test = createTestIntfPrx(adapters);
                String name = test.getAdapterName();
                final int nRetry = 10;
                int i;
                for (i = 0; i < nRetry && test.getAdapterName().equals(name); i++) {
                }
                test(i == nRetry);

                for (RemoteObjectAdapterPrx p : adapters) {
                    p.getTestIntf().ice_getConnection().close();
                }
            }

            //
            // Deactivate an adapter and ensure that we can still establish the connection to the
            // remaining adapters.
            //
            rcom.deactivateObjectAdapter(adapters.get(0));
            names.add("Adapter12");
            names.add("Adapter13");
            while (!names.isEmpty()) {
                List<RemoteObjectAdapterPrx> adpts = new ArrayList<>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                Collections.shuffle(adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                Collections.shuffle(adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.remove(test1.getAdapterName());
                test1.ice_getConnection().close();
            }

            //
            // Deactivate an adapter and ensure that we can still establish the connection to the
            // remaining adapter.
            //
            rcom.deactivateObjectAdapter(adapters.get(2));
            TestIntfPrx test = createTestIntfPrx(adapters);
            test("Adapter12".equals(test.getAdapterName()));

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing binding with multiple random endpoints... ");
        out.flush();
        {
            Random rand = new Random();

            RemoteObjectAdapterPrx[] adapters = new RemoteObjectAdapterPrx[5];
            adapters[0] = rcom.createObjectAdapter("AdapterRandom11", "default");
            adapters[1] = rcom.createObjectAdapter("AdapterRandom12", "default");
            adapters[2] = rcom.createObjectAdapter("AdapterRandom13", "default");
            adapters[3] = rcom.createObjectAdapter("AdapterRandom14", "default");
            adapters[4] = rcom.createObjectAdapter("AdapterRandom15", "default");

            boolean shortenTest =
                    System.getProperty("os.name").startsWith("Windows")
                            || System.getProperty("java.vendor").toLowerCase().indexOf("android")
                                    >= 0;

            int count = 20;
            int adapterCount = adapters.length;
            while (--count > 0) {
                TestIntfPrx[] proxies = new TestIntfPrx[10];
                if (shortenTest) {
                    if (count == 1) {
                        rcom.deactivateObjectAdapter(adapters[4]);
                        --adapterCount;
                    }
                } else {
                    if (count < 20 && count % 4 == 0) {
                        rcom.deactivateObjectAdapter(adapters[count / 4 - 1]);
                        --adapterCount;
                    }
                }

                int i;
                for (i = 0; i < proxies.length; i++) {
                    RemoteObjectAdapterPrx[] adpts =
                            new RemoteObjectAdapterPrx[rand.nextInt(adapters.length)];
                    if (adpts.length == 0) {
                        adpts = new RemoteObjectAdapterPrx[1];
                    }
                    for (int j = 0; j < adpts.length; j++) {
                        adpts[j] = adapters[rand.nextInt(adapters.length)];
                    }
                    proxies[i] = createTestIntfPrx(Arrays.asList(adpts));
                }

                for (TestIntfPrx p : proxies) {
                    p.getAdapterNameAsync();
                }
                for (TestIntfPrx p : proxies) {
                    try {
                        p.ice_ping();
                    } catch (LocalException ex) {
                    }
                }

                Set<Connection> connections = new HashSet<>();
                for (TestIntfPrx p : proxies) {
                    if (p.ice_getCachedConnection() != null) {
                        connections.add(p.ice_getCachedConnection());
                    }
                }
                test(connections.size() <= adapterCount);

                for (RemoteObjectAdapterPrx a : adapters) {
                    try {
                        a.getTestIntf().ice_getConnection().close();
                    } catch (LocalException ex) {
                        // Expected if adapter is down.
                    }
                }
            }
        }
        out.println("ok");

        out.print("testing binding with multiple endpoints and AMI... ");
        out.flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
            adapters.add(rcom.createObjectAdapter("AdapterAMI11", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI12", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI13", "default"));

            //
            // Ensure that when a connection is opened it's reused for new
            // proxies and that all endpoints are eventually tried.
            //
            Set<String> names = new HashSet<>();
            names.add("AdapterAMI11");
            names.add("AdapterAMI12");
            names.add("AdapterAMI13");
            while (!names.isEmpty()) {
                List<RemoteObjectAdapterPrx> adpts = new ArrayList<>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                Collections.shuffle(adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                Collections.shuffle(adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.remove(getAdapterNameWithAMI(test1));
                test1.ice_getConnection().close();
            }

            //
            // Ensure that the proxy correctly caches the connection (we always send the request
            // over the same connection.)
            //
            {
                for (RemoteObjectAdapterPrx p : adapters) {
                    p.getTestIntf().ice_ping();
                }

                TestIntfPrx test = createTestIntfPrx(adapters);
                String name = getAdapterNameWithAMI(test);
                final int nRetry = 10;
                int i;
                for (i = 0; i < nRetry && getAdapterNameWithAMI(test).equals(name); i++) {
                }
                test(i == nRetry);

                for (RemoteObjectAdapterPrx p : adapters) {
                    p.getTestIntf().ice_getConnection().close();
                }
            }

            //
            // Deactivate an adapter and ensure that we can still establish the connection to the
            // remaining adapters.
            //
            rcom.deactivateObjectAdapter(adapters.get(0));
            names.add("AdapterAMI12");
            names.add("AdapterAMI13");
            while (!names.isEmpty()) {
                List<RemoteObjectAdapterPrx> adpts = new ArrayList<>(adapters);

                TestIntfPrx test1 = createTestIntfPrx(adpts);
                Collections.shuffle(adpts);
                TestIntfPrx test2 = createTestIntfPrx(adpts);
                Collections.shuffle(adpts);
                TestIntfPrx test3 = createTestIntfPrx(adpts);

                test(test1.ice_getConnection() == test2.ice_getConnection());
                test(test2.ice_getConnection() == test3.ice_getConnection());

                names.remove(getAdapterNameWithAMI(test1));
                test1.ice_getConnection().close();
            }

            //
            // Deactivate an adapter and ensure that we can still establish the connection to the
            // remaining adapter.
            //
            rcom.deactivateObjectAdapter(adapters.get(2));
            TestIntfPrx test = createTestIntfPrx(adapters);
            test("AdapterAMI12".equals(getAdapterNameWithAMI(test)));

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing random endpoint selection... ");
        out.flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter21", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter22", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter23", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test(test.ice_getEndpointSelection() == EndpointSelectionType.Random);

            Set<String> names = new HashSet<>();
            names.add("Adapter21");
            names.add("Adapter22");
            names.add("Adapter23");
            while (!names.isEmpty()) {
                names.remove(test.getAdapterName());
                test.ice_getConnection().close();
            }

            test = test.ice_endpointSelection(EndpointSelectionType.Random);
            test(test.ice_getEndpointSelection() == EndpointSelectionType.Random);

            names.add("Adapter21");
            names.add("Adapter22");
            names.add("Adapter23");
            while (!names.isEmpty()) {
                names.remove(test.getAdapterName());
                test.ice_getConnection().close();
            }

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing ordered endpoint selection... ");
        out.flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter31", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter32", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter33", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test = test.ice_endpointSelection(EndpointSelectionType.Ordered);
            test(test.ice_getEndpointSelection() == EndpointSelectionType.Ordered);
            int nRetry = 5;
            int i;

            //
            // Ensure that endpoints are tried in order by deactivating the adapters
            // one after the other.
            //
            for (i = 0; i < nRetry && "Adapter31".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(0));
            for (i = 0; i < nRetry && "Adapter32".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(1));
            for (i = 0; i < nRetry && "Adapter33".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(2));

            try {
                test.getAdapterName();
            } catch (ConnectFailedException ex) {
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
            for (i = 0; i < nRetry && "Adapter36".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            test.ice_getConnection().close();
            adapters.add(rcom.createObjectAdapter("Adapter35", endpoints[1].toString()));
            for (i = 0; i < nRetry && "Adapter35".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            test.ice_getConnection().close();
            adapters.add(rcom.createObjectAdapter("Adapter34", endpoints[0].toString()));
            for (i = 0; i < nRetry && "Adapter34".equals(test.getAdapterName()); i++) {
            }
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
            try {
                test(test3.ice_getConnection() == test1.ice_getConnection());
                test(false);
            } catch (ConnectFailedException ex) {
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
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter51", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter52", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter53", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters).ice_connectionCached(false);
            test(!test.ice_isConnectionCached());

            Set<String> names = new HashSet<>();
            names.add("Adapter51");
            names.add("Adapter52");
            names.add("Adapter53");
            while (!names.isEmpty()) {
                names.remove(test.getAdapterName());
            }

            rcom.deactivateObjectAdapter(adapters.get(0));

            names.add("Adapter52");
            names.add("Adapter53");
            while (!names.isEmpty()) {
                names.remove(test.getAdapterName());
            }

            rcom.deactivateObjectAdapter(adapters.get(2));

            test("Adapter52".equals(test.getAdapterName()));

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing per request binding with multiple endpoints and AMI... ");
        out.flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
            adapters.add(rcom.createObjectAdapter("AdapterAMI51", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI52", "default"));
            adapters.add(rcom.createObjectAdapter("AdapterAMI53", "default"));

            TestIntfPrx test = createTestIntfPrx(adapters).ice_connectionCached(false);
            test(!test.ice_isConnectionCached());

            Set<String> names = new HashSet<>();
            names.add("AdapterAMI51");
            names.add("AdapterAMI52");
            names.add("AdapterAMI53");
            while (!names.isEmpty()) {
                names.remove(getAdapterNameWithAMI(test));
            }

            rcom.deactivateObjectAdapter(adapters.get(0));

            names.add("AdapterAMI52");
            names.add("AdapterAMI53");
            while (!names.isEmpty()) {
                names.remove(getAdapterNameWithAMI(test));
            }

            rcom.deactivateObjectAdapter(adapters.get(2));

            test("AdapterAMI52".equals(getAdapterNameWithAMI(test)));

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing per request binding and ordered endpoint selection... ");
        out.flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
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
            // Ensure that endpoints are tried in order by deactivating the adapters
            // one after the other.
            //
            for (i = 0; i < nRetry && "Adapter61".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(0));
            for (i = 0; i < nRetry && "Adapter62".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(1));
            for (i = 0; i < nRetry && "Adapter63".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(2));

            try {
                test.getAdapterName();
            } catch (ConnectFailedException ex) {
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
            for (i = 0; i < nRetry && "Adapter66".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            adapters.add(rcom.createObjectAdapter("Adapter65", endpoints[1].toString()));
            for (i = 0; i < nRetry && "Adapter65".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);
            adapters.add(rcom.createObjectAdapter("Adapter64", endpoints[0].toString()));
            for (i = 0; i < nRetry && "Adapter64".equals(test.getAdapterName()); i++) {
            }
            test(i == nRetry);

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing per request binding and ordered endpoint selection and AMI... ");
        out.flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
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
            // Ensure that endpoints are tried in order by deactivating the adapters
            // one after the other.
            //
            for (i = 0; i < nRetry && "AdapterAMI61".equals(getAdapterNameWithAMI(test)); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(0));
            for (i = 0; i < nRetry && "AdapterAMI62".equals(getAdapterNameWithAMI(test)); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(1));
            for (i = 0; i < nRetry && "AdapterAMI63".equals(getAdapterNameWithAMI(test)); i++) {
            }
            test(i == nRetry);
            rcom.deactivateObjectAdapter(adapters.get(2));

            try {
                test.getAdapterName();
            } catch (ConnectFailedException ex) {
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
            for (i = 0; i < nRetry && "AdapterAMI66".equals(getAdapterNameWithAMI(test)); i++) {
            }
            test(i == nRetry);
            adapters.add(rcom.createObjectAdapter("AdapterAMI65", endpoints[1].toString()));
            for (i = 0; i < nRetry && "AdapterAMI65".equals(getAdapterNameWithAMI(test)); i++) {
            }
            test(i == nRetry);
            adapters.add(rcom.createObjectAdapter("AdapterAMI64", endpoints[0].toString()));
            for (i = 0; i < nRetry && "AdapterAMI64".equals(getAdapterNameWithAMI(test)); i++) {
            }
            test(i == nRetry);

            deactivate(rcom, adapters);
        }
        out.println("ok");

        out.print("testing endpoint mode filtering... ");
        out.flush();
        {
            List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
            adapters.add(rcom.createObjectAdapter("Adapter71", "default"));
            adapters.add(rcom.createObjectAdapter("Adapter72", "udp"));

            TestIntfPrx test = createTestIntfPrx(adapters);
            test("Adapter71".equals(test.getAdapterName()));

            TestIntfPrx testUDP = test.ice_datagram();
            test(test.ice_getConnection() != testUDP.ice_getConnection());
            try {
                testUDP.getAdapterName();
            } catch (TwowayOnlyException ex) {
            }
        }
        out.println("ok");

        if ("ssl".equals(communicator.getProperties().getIceProperty("Ice.Default.Protocol"))) {
            out.print("testing unsecure vs. secure endpoints... ");
            out.flush();
            {
                List<RemoteObjectAdapterPrx> adapters = new ArrayList<>();
                adapters.add(rcom.createObjectAdapter("Adapter81", "ssl"));
                adapters.add(rcom.createObjectAdapter("Adapter82", "tcp"));

                TestIntfPrx test = createTestIntfPrx(adapters);
                int i;
                for (i = 0; i < 5; i++) {
                    test("Adapter82".equals(test.getAdapterName()));
                    test.ice_getConnection().close();
                }

                TestIntfPrx testSecure = test.ice_secure(true);
                test(testSecure.ice_isSecure());
                testSecure = test.ice_secure(false);
                test(!testSecure.ice_isSecure());
                testSecure = test.ice_secure(true);
                test(testSecure.ice_isSecure());
                test(test.ice_getConnection() != testSecure.ice_getConnection());

                rcom.deactivateObjectAdapter(adapters.get(1));

                for (i = 0; i < 5; i++) {
                    test("Adapter81".equals(test.getAdapterName()));
                    test.ice_getConnection().close();
                }

                rcom.createObjectAdapter(
                        "Adapter83", (test.ice_getEndpoints()[1]).toString()); // Reactive tcp OA.

                for (i = 0; i < 5; i++) {
                    test("Adapter83".equals(test.getAdapterName()));
                    test.ice_getConnection().close();
                }

                rcom.deactivateObjectAdapter(adapters.get(0));
                try {
                    testSecure.ice_ping();
                    test(false);
                } catch (ConnectFailedException ex) {
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

            Properties ipv4 = new Properties();
            ipv4.setProperty("Ice.IPv4", "1");
            ipv4.setProperty("Ice.IPv6", "0");
            ipv4.setProperty("Adapter.Endpoints", "tcp -h localhost");

            Properties ipv6 = new Properties();
            ipv6.setProperty("Ice.IPv4", "0");
            ipv6.setProperty("Ice.IPv6", "1");
            ipv6.setProperty("Adapter.Endpoints", "tcp -h localhost");

            Properties bothPreferIPv4 = new Properties();
            bothPreferIPv4.setProperty("Ice.IPv4", "1");
            bothPreferIPv4.setProperty("Ice.IPv6", "1");
            bothPreferIPv4.setProperty("Ice.PreferIPv6Address", "0");
            bothPreferIPv4.setProperty("Adapter.Endpoints", "tcp -h localhost");

            Properties bothPreferIPv6 = new Properties();
            bothPreferIPv6.setProperty("Ice.IPv4", "1");
            bothPreferIPv6.setProperty("Ice.IPv6", "1");
            bothPreferIPv6.setProperty("Ice.PreferIPv6Address", "1");
            bothPreferIPv6.setProperty("Adapter.Endpoints", "tcp -h localhost");

            List<Properties> clientProps = new ArrayList<>();
            clientProps.add(ipv4);
            clientProps.add(ipv6);
            clientProps.add(bothPreferIPv4);
            clientProps.add(bothPreferIPv6);

            String endpoint = "tcp -p " + helper.getTestPort(2);

            Properties anyipv4 = ipv4._clone();
            anyipv4.setProperty("Adapter.Endpoints", endpoint);
            anyipv4.setProperty("Adapter.PublishedEndpoints", endpoint + " -h 127.0.0.1");

            Properties anyipv6 = ipv6._clone();
            anyipv6.setProperty("Adapter.Endpoints", endpoint);
            anyipv6.setProperty("Adapter.PublishedEndpoints", endpoint + " -h \".1\"");

            Properties anyboth = new Properties();
            anyboth.setProperty("Ice.IPv4", "1");
            anyboth.setProperty("Ice.IPv6", "1");
            anyboth.setProperty("Adapter.Endpoints", endpoint);
            anyboth.setProperty(
                    "Adapter.PublishedEndpoints",
                    endpoint + " -h \"::1\":" + endpoint + " -h 127.0.0.1");

            Properties localipv4 = ipv4._clone();
            localipv4.setProperty("Adapter.Endpoints", "tcp -h 127.0.0.1");

            Properties localipv6 = ipv6._clone();
            localipv6.setProperty("Adapter.Endpoints", "tcp -h \"::1\"");

            List<Properties> serverProps =
                    new ArrayList<>(clientProps);
            serverProps.add(anyipv4);
            serverProps.add(anyipv6);
            serverProps.add(anyboth);
            serverProps.add(localipv4);
            serverProps.add(localipv6);

            boolean ipv6NotSupported = false;
            for (Properties p : serverProps) {
                try (Communicator serverCommunicator = helper.initialize(p)) {
                    ObjectAdapter oa;
                    try {
                        oa = serverCommunicator.createObjectAdapter("Adapter");
                        oa.activate();
                    } catch (DNSException ex) {
                        serverCommunicator.destroy();
                        continue; // IP version not supported.
                    } catch (SocketException ex) {
                        if (p == ipv6) {
                            ipv6NotSupported = true;
                        }
                        serverCommunicator.destroy();
                        continue; // IP version not supported.
                    }

                    String strPrx =
                            oa.createProxy(Util.stringToIdentity("dummy")).toString();
                    for (Properties q : clientProps) {
                        try (Communicator clientCommunicator = helper.initialize(q)) {
                            ObjectPrx prx = clientCommunicator.stringToProxy(strPrx);
                            try {
                                prx.ice_ping();
                                test(false);
                            } catch (ObjectNotExistException ex) {
                                // Expected, no object registered.
                            } catch (DNSException ex) {
                                // Expected if no IPv4 or IPv6 address is associated to localhost or
                                // if trying to connect
                                // to an any endpoint with the wrong IP version,
                                // e.g.: resolving an IPv4 address when only IPv6 is enabled fails
                                // with a DNS exception.
                            } catch (SocketException ex) {
                                test(
                                        (p == ipv4 && q == ipv6)
                                                || (p == ipv6 && q == ipv4)
                                                || (p == bothPreferIPv4 && q == ipv6)
                                                || (p == bothPreferIPv6 && q == ipv4)
                                                || (p == bothPreferIPv6
                                                        && q == ipv6
                                                        && ipv6NotSupported)
                                                || (p == anyipv4 && q == ipv6)
                                                || (p == anyipv6 && q == ipv4)
                                                || (p == localipv4 && q == ipv6)
                                                || (p == localipv6 && q == ipv4)
                                                || (p == ipv6 && q == bothPreferIPv4)
                                                || (p == bothPreferIPv6 && q == ipv6)
                                                || (p == ipv6 && q == bothPreferIPv4)
                                                || (p == ipv6 && q == bothPreferIPv6)
                                                || (p == bothPreferIPv6 && q == ipv6));
                            }
                        }
                    }
                }
            }
            out.println("ok");
        }

        rcom.shutdown();
    }

    private AllTests() {
    }
}
