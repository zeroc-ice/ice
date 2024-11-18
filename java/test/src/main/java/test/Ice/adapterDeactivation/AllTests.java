//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.adapterDeactivation;

import com.zeroc.Ice.EndpointInfo;
import com.zeroc.Ice.IPEndpointInfo;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.adapterDeactivation.Test.TestIntfPrx;

import java.util.Arrays;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static EndpointInfo getUnderlying(EndpointInfo endpointInfo) {
        return endpointInfo.underlying == null
                ? endpointInfo
                : getUnderlying(endpointInfo.underlying);
    }

    public static TestIntfPrx allTests(test.TestHelper helper) {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        java.io.PrintWriter out = helper.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrx.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        {
            out.print("creating/destroying/recreating object adapter... ");
            out.flush();
            com.zeroc.Ice.ObjectAdapter adapter =
                    communicator.createObjectAdapterWithEndpoints(
                            "TransientTestAdapter", "default");
            try {
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                test(false);
            } catch (com.zeroc.Ice.AlreadyRegisteredException ex) {
            }
            adapter.destroy();
            //
            // Use a different port than the first adapter to avoid an "address already in use"
            // error.
            //
            adapter =
                    communicator.createObjectAdapterWithEndpoints(
                            "TransientTestAdapter", "default");
            adapter.destroy();
            out.println("ok");
        }

        out.print("creating/activating/deactivating object adapter in one operation... ");
        out.flush();
        obj._transient();
        obj.transientAsync().join();
        out.println("ok");

        {
            out.print("testing connection closure... ");
            out.flush();
            for (int i = 0; i < 10; ++i) {
                try (com.zeroc.Ice.Communicator comm =
                        helper.initialize(communicator.getProperties()._clone())) {
                    comm.stringToProxy("test:" + helper.getTestEndpoint(0)).ice_pingAsync();
                }
            }
            out.println("ok");
        }

        out.print("testing object adapter published endpoints... ");
        out.flush();
        {
            communicator
                    .getProperties()
                    .setProperty(
                            "PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("PAdapter");
            test(adapter.getPublishedEndpoints().length == 1);
            com.zeroc.Ice.Endpoint endpt = adapter.getPublishedEndpoints()[0];
            test(endpt.toString().equals("tcp -h localhost -p 12345 -t 30000"));
            com.zeroc.Ice.ObjectPrx prx =
                    communicator.stringToProxy(
                            "dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
            adapter.setPublishedEndpoints(prx.ice_getEndpoints());
            test(adapter.getPublishedEndpoints().length == 2);
            com.zeroc.Ice.Identity id = new com.zeroc.Ice.Identity();
            id.name = "dummy";
            test(Arrays.equals(adapter.createProxy(id).ice_getEndpoints(), prx.ice_getEndpoints()));
            test(Arrays.equals(adapter.getPublishedEndpoints(), prx.ice_getEndpoints()));
            adapter.destroy();
            test(adapter.getPublishedEndpoints().length == 0);
        }
        out.println("ok");

        out.print("testing object adapter published host... ");
        out.flush();
        {
            communicator.getProperties().setProperty("PHAdapter.Endpoints", "default -h *");

            // PublishedHost not set
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                test(publishedEndpoints.length == 1);
                IPEndpointInfo ipEndpointInfo =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                test(ipEndpointInfo.host.length() > 0);
                adapter.destroy();
            }

            communicator.getProperties().setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                test(publishedEndpoints.length == 1);
                IPEndpointInfo ipEndpointInfo =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                test(ipEndpointInfo.host.equals("test.zeroc.com"));
                adapter.destroy();
            }

            // Listening on loopback
            communicator.getProperties().setProperty("PHAdapter.Endpoints", "default -h 127.0.0.1");

            communicator.getProperties().setProperty("PHAdapter.PublishedHost", "");
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                test(publishedEndpoints.length == 1);
                IPEndpointInfo ipEndpointInfo =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                test(ipEndpointInfo.host.equals("127.0.0.1"));
                adapter.destroy();
            }

            communicator.getProperties().setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                IPEndpointInfo ipEndpointInfo =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                test(ipEndpointInfo.host.equals("test.zeroc.com"));
                adapter.destroy();
            }

            // Two loopback endpoints with different ports
            communicator
                    .getProperties()
                    .setProperty(
                            "PHAdapter.Endpoints",
                            "default -h 127.0.0.1 -p 12345:default -h 127.0.0.1");

            communicator.getProperties().setProperty("PHAdapter.PublishedHost", "");
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                test(publishedEndpoints.length == 2);
                IPEndpointInfo ipEndpointInfo0 =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                IPEndpointInfo ipEndpointInfo1 =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[1].getInfo());

                test(ipEndpointInfo0.host.equals("127.0.0.1") && ipEndpointInfo0.port == 12345);
                test(ipEndpointInfo1.host.equals("127.0.0.1") && ipEndpointInfo1.port != 12345);
                adapter.destroy();
            }

            // Two endpoints - one loopback, one not loopback
            communicator
                    .getProperties()
                    .setProperty(
                            "PHAdapter.Endpoints", "default -h 127.0.0.1 -p 12345:default -h *");

            communicator.getProperties().setProperty("PHAdapter.PublishedHost", "");
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                test(publishedEndpoints.length == 1); // loopback filtered out
                IPEndpointInfo ipEndpointInfo =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                test(ipEndpointInfo.host.length() > 0 && ipEndpointInfo.port != 12345);
                adapter.destroy();
            }

            communicator.getProperties().setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                test(publishedEndpoints.length == 1); // loopback filtered out
                IPEndpointInfo ipEndpointInfo =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                test(ipEndpointInfo.host.equals("test.zeroc.com") && ipEndpointInfo.port != 12345);
                adapter.destroy();
            }

            // Two non-loopback endpoints
            communicator
                    .getProperties()
                    .setProperty("PHAdapter.Endpoints", "tcp -h * -p 12345:default -h *");

            communicator.getProperties().setProperty("PHAdapter.PublishedHost", "");
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                test(publishedEndpoints.length == 2);
                IPEndpointInfo ipEndpointInfo0 =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                IPEndpointInfo ipEndpointInfo1 =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[1].getInfo());
                test(ipEndpointInfo0.host.length() > 0 && ipEndpointInfo0.port == 12345);
                test(ipEndpointInfo1.host.length() > 0 && ipEndpointInfo1.port != 12345);
                adapter.destroy();
            }

            communicator.getProperties().setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
            {
                ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                com.zeroc.Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
                test(publishedEndpoints.length == 2);
                IPEndpointInfo ipEndpointInfo0 =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[0].getInfo());
                IPEndpointInfo ipEndpointInfo1 =
                        (IPEndpointInfo) getUnderlying(publishedEndpoints[1].getInfo());
                test(
                        ipEndpointInfo0.host.equals("test.zeroc.com")
                                && ipEndpointInfo0.port == 12345);
                test(
                        ipEndpointInfo1.host.equals("test.zeroc.com")
                                && ipEndpointInfo1.port != 12345);
                adapter.destroy();
            }
        }
        out.println("ok");

        if (obj.ice_getConnection() != null) {
            out.print("testing object adapter with bi-dir connection... ");
            out.flush();

            test(communicator.getDefaultObjectAdapter() == null);
            test(obj.ice_getCachedConnection().getAdapter() == null);

            ObjectAdapter adapter = communicator.createObjectAdapter("");

            communicator.setDefaultObjectAdapter(adapter);
            test(communicator.getDefaultObjectAdapter() == adapter);

            // create new connection
            obj.ice_getCachedConnection().close();
            obj.ice_ping();

            test(obj.ice_getCachedConnection().getAdapter() == adapter);
            communicator.setDefaultObjectAdapter(null);

            // create new connection
            obj.ice_getCachedConnection().close();
            obj.ice_ping();

            test(obj.ice_getCachedConnection().getAdapter() == null);
            obj.ice_getCachedConnection().setAdapter(adapter);
            test(obj.ice_getCachedConnection().getAdapter() == adapter);
            obj.ice_getCachedConnection().setAdapter(null);

            adapter.destroy();
            try {
                obj.ice_getConnection().setAdapter(adapter);
                test(false);
            } catch (com.zeroc.Ice.ObjectAdapterDestroyedException ex) {
            }
            out.println("ok");
        }

        out.print("testing object adapter with router... ");
        out.flush();
        {
            com.zeroc.Ice.Identity routerId = new com.zeroc.Ice.Identity();
            routerId.name = "router";
            com.zeroc.Ice.RouterPrx router =
                    com.zeroc.Ice.RouterPrx.uncheckedCast(
                            base.ice_identity(routerId).ice_connectionId("rc"));
            com.zeroc.Ice.ObjectAdapter adapter =
                    communicator.createObjectAdapterWithRouter("", router);

            test(adapter.getPublishedEndpoints().length == 1);
            test(
                    adapter.getPublishedEndpoints()[0]
                            .toString()
                            .equals("tcp -h localhost -p 23456 -t 30000"));
            try {
                adapter.setPublishedEndpoints(router.ice_getEndpoints());
                test(false);
            } catch (IllegalArgumentException ex) {
                // Expected.
            }

            adapter.destroy();

            try {
                routerId.name = "test";
                router = com.zeroc.Ice.RouterPrx.uncheckedCast(base.ice_identity(routerId));
                communicator.createObjectAdapterWithRouter("", router);
                test(false);
            } catch (com.zeroc.Ice.OperationNotExistException ex) {
                // Expected: the "test" object doesn't implement Ice::Router!
            }
            try {
                router =
                        com.zeroc.Ice.RouterPrx.createProxy(
                                communicator, "test:" + helper.getTestEndpoint(1));
                communicator.createObjectAdapterWithRouter("", router);
                test(false);
            } catch (com.zeroc.Ice.ConnectFailedException ex) {
            }
            try {
                router =
                        com.zeroc.Ice.RouterPrx.createProxy(
                                communicator, "test:" + helper.getTestEndpoint(1));

                communicator
                        .getProperties()
                        .setProperty("AdapterWithRouter.Endpoints", "tcp -h 127.0.0.1");
                communicator.createObjectAdapterWithRouter("AdapterWithRouter", router);
                test(false);
            } catch (com.zeroc.Ice.InitializationException ex) {
            }
        }
        out.println("ok");

        out.print("testing object adapter creation with port in use... ");
        out.flush();
        {
            com.zeroc.Ice.ObjectAdapter adapter1 =
                    communicator.createObjectAdapterWithEndpoints(
                            "Adpt1", helper.getTestEndpoint(10));
            try {
                communicator.createObjectAdapterWithEndpoints("Adpt2", helper.getTestEndpoint(10));
                test(false);
            } catch (com.zeroc.Ice.LocalException ex) {
                // Expected can't re-use the same endpoint.
            }
            adapter1.destroy();
        }
        out.println("ok");

        out.print("deactivating object adapter in the server... ");
        out.flush();
        obj.deactivate();
        out.println("ok");

        out.print("testing whether server is gone... ");
        out.flush();
        if (obj.ice_getConnection() == null) { // collocated
            obj.ice_ping(); // works fine
            out.println("ok");
            out.flush();
        } else {
            try {
                obj.ice_invocationTimeout(100)
                        .ice_ping(); // Use timeout to speed up testing on Windows
                test(false);
            } catch (com.zeroc.Ice.LocalException ex) {
                out.println("ok");
                out.flush();
            }
        }

        out.print("testing server idle time...");
        out.flush();
        Thread thread1 =
                new Thread(
                        () -> {
                            com.zeroc.Ice.InitializationData initData =
                                    new com.zeroc.Ice.InitializationData();
                            initData.properties = new com.zeroc.Ice.Properties();
                            initData.properties.setProperty("Ice.ServerIdleTime", "1");
                            try (com.zeroc.Ice.Communicator idleCommunicator =
                                    com.zeroc.Ice.Util.initialize(initData)) {
                                com.zeroc.Ice.ObjectAdapter adapter =
                                        idleCommunicator.createObjectAdapterWithEndpoints(
                                                "IdleAdapter", "tcp -h 127.0.0.1");
                                adapter.activate();
                                idleCommunicator.waitForShutdown();
                            }
                        });
        Thread thread2 =
                new Thread(
                        () -> {
                            com.zeroc.Ice.InitializationData initData =
                                    new com.zeroc.Ice.InitializationData();
                            initData.properties = new com.zeroc.Ice.Properties();
                            initData.properties.setProperty("Ice.ServerIdleTime", "0");
                            try (com.zeroc.Ice.Communicator idleCommunicator =
                                    com.zeroc.Ice.Util.initialize(initData)) {
                                com.zeroc.Ice.ObjectAdapter adapter =
                                        idleCommunicator.createObjectAdapterWithEndpoints(
                                                "IdleAdapter", "tcp -h 127.0.0.1");
                                adapter.activate();
                                try {
                                    Thread.sleep(1200);
                                } catch (InterruptedException ex) {
                                    test(false);
                                }
                                test(!idleCommunicator.isShutdown());
                            }
                        });
        thread1.start();
        thread2.start();
        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException ex) {
            test(false);
        }
        out.println("ok");

        return obj;
    }
}
