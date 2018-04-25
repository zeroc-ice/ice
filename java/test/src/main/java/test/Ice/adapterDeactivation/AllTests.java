// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

import java.util.Arrays;
import test.Ice.adapterDeactivation.Test.TestIntfPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static TestIntfPrx allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        java.io.PrintWriter out = app.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test:" + app.getTestEndpoint(0);
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
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            try
            {
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                test(false);
            }
            catch(com.zeroc.Ice.AlreadyRegisteredException ex)
            {
            }
            adapter.destroy();
            //
            // Use a different port than the first adapter to avoid an "address already in use" error.
            //
            adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
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
            for(int i = 0; i < 10; ++i)
            {
                com.zeroc.Ice.InitializationData initData = app.createInitializationData();
                initData.properties = communicator.getProperties()._clone();
                com.zeroc.Ice.Communicator comm = app.initialize(initData);
                comm.stringToProxy("test:" + app.getTestEndpoint(0)).ice_pingAsync();
                comm.destroy();
            }
            out.println("ok");
        }

        out.print("testing object adapter published endpoints... ");
        out.flush();
        {
            communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("PAdapter");
            test(adapter.getPublishedEndpoints().length == 1);
            com.zeroc.Ice.Endpoint endpt = adapter.getPublishedEndpoints()[0];
            test(endpt.toString().equals("tcp -h localhost -p 12345 -t 30000"));
            com.zeroc.Ice.ObjectPrx prx =
                communicator.stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
            adapter.setPublishedEndpoints(prx.ice_getEndpoints());
            test(adapter.getPublishedEndpoints().length == 2);
            com.zeroc.Ice.Identity id = new com.zeroc.Ice.Identity();
            id.name = "dummy";
            test(Arrays.equals(adapter.createProxy(id).ice_getEndpoints(), prx.ice_getEndpoints()));
            test(Arrays.equals(adapter.getPublishedEndpoints(), prx.ice_getEndpoints()));
            adapter.refreshPublishedEndpoints();
            test(adapter.getPublishedEndpoints().length == 1);
            test(adapter.getPublishedEndpoints()[0].equals(endpt));
            communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
            adapter.refreshPublishedEndpoints();
            test(adapter.getPublishedEndpoints().length == 1);
            test(adapter.getPublishedEndpoints()[0].toString().equals("tcp -h localhost -p 12345 -t 20000"));
            adapter.destroy();
            test(adapter.getPublishedEndpoints().length == 0);
        }
        out.println("ok");

        if(obj.ice_getConnection() != null)
        {
            out.print("testing object adapter with bi-dir connection... ");
            out.flush();
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("");
            obj.ice_getConnection().setAdapter(adapter);
            obj.ice_getConnection().setAdapter(null);
            adapter.deactivate();
            try
            {
                obj.ice_getConnection().setAdapter(adapter);
                test(false);
            }
            catch(com.zeroc.Ice.ObjectAdapterDeactivatedException ex)
            {
            }
            out.println("ok");
        }

        out.print("testing object adapter with router... ");
        out.flush();
        {
            com.zeroc.Ice.Identity routerId = new com.zeroc.Ice.Identity();
            routerId.name = "router";
            com.zeroc.Ice.RouterPrx router =
                com.zeroc.Ice.RouterPrx.uncheckedCast(base.ice_identity(routerId).ice_connectionId("rc"));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithRouter("", router);
            test(adapter.getPublishedEndpoints().length == 1);
            test(adapter.getPublishedEndpoints()[0].toString().equals("tcp -h localhost -p 23456 -t 30000"));
            adapter.refreshPublishedEndpoints();
            test(adapter.getPublishedEndpoints().length == 1);
            test(adapter.getPublishedEndpoints()[0].toString().equals("tcp -h localhost -p 23457 -t 30000"));
            try
            {
                adapter.setPublishedEndpoints(router.ice_getEndpoints());
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
                // Expected.
            }
            adapter.destroy();

            try
            {
                routerId.name = "test";
                router = com.zeroc.Ice.RouterPrx.uncheckedCast(base.ice_identity(routerId));
                communicator.createObjectAdapterWithRouter("", router);
                test(false);
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
                // Expected: the "test" object doesn't implement Ice::Router!
            }

            try
            {
                router =
                    com.zeroc.Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("test:" + app.getTestEndpoint(1)));
                communicator.createObjectAdapterWithRouter("", router);
                test(false);
            }
            catch(com.zeroc.Ice.ConnectFailedException ex)
            {
            }
        }
        out.println("ok");

        out.print("testing object adapter creation with port in use... ");
        out.flush();
        {
            com.zeroc.Ice.ObjectAdapter adapter1 =
                communicator.createObjectAdapterWithEndpoints("Adpt1", app.getTestEndpoint(10));
            try
            {
                communicator.createObjectAdapterWithEndpoints("Adpt2", app.getTestEndpoint(10));
                test(false);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
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
        try
        {
            obj.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
            throw new RuntimeException();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            out.println("ok");
        }

        return obj;
    }
}
