// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public class AllTests : Test.AllTests
{
    public static TestIntfPrx allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        Write("testing stringToProxy... ");
        Flush();
        string @ref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        test(obj.Equals(@base));
        WriteLine("ok");

        {
            Write("creating/destroying/recreating object adapter... ");
            Flush();
            Ice.ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            try
            {
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                test(false);
            }
            catch (Ice.AlreadyRegisteredException)
            {
            }
            adapter.destroy();

            //
            // Use a different port than the first adapter to avoid an "address already in use" error.
            //
            adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            adapter.destroy();
            Console.Out.WriteLine("ok");
        }

        Write("creating/activating/deactivating object adapter in one operation... ");
        Flush();
        obj.transient();
        obj.end_transient(obj.begin_transient());
        WriteLine("ok");

        {
            Write("testing connection closure... ");
            Flush();
            for(int i = 0; i < 10; ++i)
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().ice_clone_();
                Ice.Communicator comm = Ice.Util.initialize(initData);
                comm.stringToProxy("test:" + helper.getTestEndpoint(0)).begin_ice_ping();
                comm.destroy();
            }
            WriteLine("ok");
        }

        Write("testing object adapter published endpoints... ");
        Flush();
        {
            communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("PAdapter");
            test(adapter.getPublishedEndpoints().Length == 1);
            Ice.Endpoint endpt = adapter.getPublishedEndpoints()[0];
            test(endpt.ToString().Equals("tcp -h localhost -p 12345 -t 30000"));
            Ice.ObjectPrx prx =
                communicator.stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
            adapter.setPublishedEndpoints(prx.ice_getEndpoints());
            test(adapter.getPublishedEndpoints().Length == 2);
            Ice.Identity id = new Ice.Identity();
            id.name = "dummy";
            test(IceUtilInternal.Arrays.Equals(adapter.createProxy(id).ice_getEndpoints(), prx.ice_getEndpoints()));
            test(IceUtilInternal.Arrays.Equals(adapter.getPublishedEndpoints(), prx.ice_getEndpoints()));
            adapter.refreshPublishedEndpoints();
            test(adapter.getPublishedEndpoints().Length == 1);
            test(adapter.getPublishedEndpoints()[0].Equals(endpt));
            communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
            adapter.refreshPublishedEndpoints();
            test(adapter.getPublishedEndpoints().Length == 1);
            test(adapter.getPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 12345 -t 20000"));
            adapter.destroy();
            test(adapter.getPublishedEndpoints().Length == 0);
        }
        WriteLine("ok");

        if(obj.ice_getConnection() != null)
        {
            Write("testing object adapter with bi-dir connection... ");
            Flush();
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("");
            obj.ice_getConnection().setAdapter(adapter);
            obj.ice_getConnection().setAdapter(null);
            adapter.deactivate();
            try
            {
                obj.ice_getConnection().setAdapter(adapter);
                test(false);
            }
            catch(Ice.ObjectAdapterDeactivatedException)
            {
            }
            WriteLine("ok");
        }

        Write("testing object adapter with router... ");
        Flush();
        {
            Ice.Identity routerId = new Ice.Identity();
            routerId.name = "router";
            Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(@base.ice_identity(routerId).ice_connectionId("rc"));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithRouter("", router);
            test(adapter.getPublishedEndpoints().Length == 1);
            test(adapter.getPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 23456 -t 30000"));
            adapter.refreshPublishedEndpoints();
            test(adapter.getPublishedEndpoints().Length == 1);
            test(adapter.getPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 23457 -t 30000"));
            try
            {
                adapter.setPublishedEndpoints(router.ice_getEndpoints());
                test(false);
            }
            catch(ArgumentException)
            {
                // Expected.
            }
            adapter.destroy();

            try
            {
                routerId.name = "test";
                router = Ice.RouterPrxHelper.uncheckedCast(@base.ice_identity(routerId));
                communicator.createObjectAdapterWithRouter("", router);
                test(false);
            }
            catch(Ice.OperationNotExistException)
            {
                // Expected: the "test" object doesn't implement Ice::Router!
            }

            try
            {
                router = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("test:" +
                                                                                      helper.getTestEndpoint(1)));
                communicator.createObjectAdapterWithRouter("", router);
                test(false);
            }
            catch(Ice.ConnectFailedException)
            {
            }
        }
        WriteLine("ok");

        Write("testing object adapter creation with port in use... ");
        Flush();
        {
            var adapter1 = communicator.createObjectAdapterWithEndpoints("Adpt1", helper.getTestEndpoint(10));
            try
            {
                communicator.createObjectAdapterWithEndpoints("Adpt2", helper.getTestEndpoint(10));
                test(false);
            }
            catch(Ice.LocalException)
            {
                // Expected can't re-use the same endpoint.
            }
            adapter1.destroy();
        }
        WriteLine("ok");

        Write("deactivating object adapter in the server... ");
        Flush();
        obj.deactivate();
        WriteLine("ok");

        Write("testing whether server is gone... ");
        Flush();
        try
        {
            obj.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
            test(false);
        }
        catch(Ice.LocalException)
        {
            WriteLine("ok");
        }

        return obj;
    }
}
