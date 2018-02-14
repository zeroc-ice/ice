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
    allTests(test.Util.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        java.io.PrintWriter out = app.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test:" + app.getTestEndpoint(0);
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
                Ice.InitializationData initData = app.createInitializationData();
                initData.properties = communicator.getProperties()._clone();
                Ice.Communicator comm = app.initialize(initData);
                comm.stringToProxy("test:" + app.getTestEndpoint(0)).begin_ice_ping();
                comm.destroy();
            }
            out.println("ok");
        }

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
