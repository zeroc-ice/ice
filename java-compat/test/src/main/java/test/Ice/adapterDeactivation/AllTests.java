// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

import test.Ice.adapterDeactivation.Test.TestIntfPrx;
import test.Ice.adapterDeactivation.Test.TestIntfPrxHelper;
import java.util.Arrays;

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

    public static TestIntfPrx
    allTests(test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        java.io.PrintWriter out = helper.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        {
            out.print("creating/destroying/recreating object adapter... ");
            out.flush();
            Ice.ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            try
            {
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException ex)
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
        obj.end_transient(obj.begin_transient());
        out.println("ok");

        {
            out.print("testing connection closure... ");
            out.flush();
            for(int i = 0; i < 10; ++i)
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties()._clone();
                try(Ice.Communicator comm = helper.initialize(initData, false))
                {
                    comm.stringToProxy("test:" + helper.getTestEndpoint(0)).begin_ice_ping();
                }
            }
            out.println("ok");
        }

        out.print("testing object adapter published endpoints... ");
        out.flush();
        {
            communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("PAdapter");
            test(adapter.getPublishedEndpoints().length == 1);
            Ice.Endpoint endpt = adapter.getPublishedEndpoints()[0];
            test(endpt.toString().equals("tcp -h localhost -p 12345 -t 30000"));
            Ice.ObjectPrx prx =
                communicator.stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
            adapter.setPublishedEndpoints(prx.ice_getEndpoints());
            test(adapter.getPublishedEndpoints().length == 2);
            Ice.Identity id = new Ice.Identity();
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
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("");
            obj.ice_getConnection().setAdapter(adapter);
            obj.ice_getConnection().setAdapter(null);
            adapter.deactivate();
            try
            {
                obj.ice_getConnection().setAdapter(adapter);
                test(false);
            }
            catch(Ice.ObjectAdapterDeactivatedException ex)
            {
            }
            out.println("ok");
        }

        out.print("testing object adapter with router... ");
        out.flush();
        {
            Ice.Identity routerId = new Ice.Identity();
            routerId.name = "router";
            Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(base.ice_identity(routerId).ice_connectionId("rc"));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithRouter("", router);
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
                router = Ice.RouterPrxHelper.uncheckedCast(base.ice_identity(routerId));
                communicator.createObjectAdapterWithRouter("", router);
                test(false);
            }
            catch(Ice.OperationNotExistException ex)
            {
                // Expected: the "test" object doesn't implement Ice::Router!
            }

            try
            {
                router =
                    Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("test:" + helper.getTestEndpoint(1)));
                communicator.createObjectAdapterWithRouter("", router);
                test(false);
            }
            catch(Ice.ConnectFailedException ex)
            {
            }
        }
        out.println("ok");

        out.print("testing object adapter creation with port in use... ");
        out.flush();
        {
            Ice.ObjectAdapter adapter1 =
                communicator.createObjectAdapterWithEndpoints("Adpt1", helper.getTestEndpoint(10));
            try
            {
                communicator.createObjectAdapterWithEndpoints("Adpt2", helper.getTestEndpoint(10));
                test(false);
            }
            catch(Ice.LocalException ex)
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
        catch(Ice.LocalException ex)
        {
            out.println("ok");
        }

        return obj;
    }
}
