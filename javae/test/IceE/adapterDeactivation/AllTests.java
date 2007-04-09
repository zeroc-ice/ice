// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = communicator.getProperties().getPropertyWithDefault("Test.Proxy", 
		"test:default -p 12010 -t 10000");
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

            adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9998");
            adapter.destroy();
            out.println("ok");

            //
            // Resources on J2ME devices aren't always release in a very
            // timely manner.  We explicitly call for a garbage collection
            // with the hope that the KVM will serially release all actual
            // resources before continuing with the test to prevent false
            // negatives. This seems to work on the Nokia 6230i.
            //
            System.gc();
        }

        out.print("creating/activating/deactivating object adapter in one operation... ");
        out.flush();
        obj._transient();
        out.println("ok");

        out.print("deactivating object adapter in the server... ");
        out.flush();
        obj.deactivate();
        out.println("ok");

        out.print("testing whether server is gone... ");
        out.flush();
        try
        {
            obj.ice_ping();
            throw new RuntimeException();
        }
        catch(Ice.LocalException ex)
        {
            out.println("ok");
        }

        return obj;
    }
}
