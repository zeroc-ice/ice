// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

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
    allTests(Ice.Communicator communicator, int[] ports)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test";
        for(int i = 0; i < ports.length; i++)
        {
            ref += ":default -t 4000 -p " + ports[i];
        }
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestPrx obj = TestPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        System.out.println("ok");

        int oldPid = 0;
        for(int i = 1, j = 0; i <= ports.length; ++i, j = j >= 3 ? 0 : j + 1)
        {
            System.out.print("testing server #" + i + "... ");
            System.out.flush();
            int pid = obj.pid();
            test(pid != oldPid);
            System.out.println("ok");

            if(j == 0)
            {
                System.out.print("shutting down server #" + i + "... ");
                System.out.flush();
                obj.shutdown();
                System.out.println("ok");
            }
            else if(j == 1 || i + 1 > ports.length)
            {
                System.out.print("aborting server #" + i + "... ");
                System.out.flush();
                try
                {
                    obj.abort();
                    test(false);
                }
                catch(Ice.ConnectionLostException ex)
                {
                    System.out.println("ok");
                }
            }
            else if(j == 2)
            {
                System.out.print("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
                System.out.flush();
                try
                {
                    obj.idempotentAbort();
                    test(false);
                }
                catch(Ice.ConnectionLostException ex)
                {
                    System.out.println("ok");
                }
                ++i;
            }
            else if(j == 3)
            {
                System.out.print("aborting server #" + i + " and #" + (i + 1) + " with nonmutating call... ");
                System.out.flush();
                try
                {
                    obj.nonmutatingAbort();
                    test(false);
                }
                catch(Ice.ConnectionLostException ex)
                {
                    System.out.println("ok");
                }
                ++i;
            }
            else
            {
                assert(false);
            }
        }

        System.out.print("testing whether all servers are gone... ");
        System.out.flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
        }
    }
}
