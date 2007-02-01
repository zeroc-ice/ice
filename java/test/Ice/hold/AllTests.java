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

    public static void
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "hold:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");
        
        System.out.print("testing checked cast... ");
        System.out.flush();
        HoldPrx hold = HoldPrxHelper.checkedCast(base);
        test(hold != null);
        test(hold.equals(base));
        System.out.println("ok");
        
        System.out.print("changing state between active and hold rapidly... ");
        System.out.flush();
        for(int i = 0; i < 100; ++i)
        {
            hold.putOnHold(0);
        }
        System.out.println("ok");
        
        System.out.print("changing state to hold and shutting down server... ");
        System.out.flush();
        hold.shutdown();
        System.out.println("ok");
    }
}
        