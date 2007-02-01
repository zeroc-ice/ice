// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        System.out.println("ok");

        {
            System.out.print("creating/destroying/recreating object adapter... ");
            System.out.flush();
            Ice.ObjectAdapter adapter = 
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999");
            try
            {
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9998");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException ex)
            {
            }
            adapter.destroy();
            //
            // Use a different port than the first adapter to avoid an "address already in use" error.
            //
            adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9998");
            adapter.destroy();
            System.out.println("ok");
        }

        System.out.print("creating/activating/deactivating object adapter in one operation... ");
        System.out.flush();
        obj._transient();
        System.out.println("ok");

        System.out.print("deactivating object adapter in the server... ");
        System.out.flush();
        obj.deactivate();
        System.out.println("ok");

        System.out.print("testing whether server is gone... ");
        System.out.flush();
        try
        {
            obj.ice_ping();
            throw new RuntimeException();
        }
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
        }

        return obj;
    }
}
