// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    allTests(Ice.Communicator communicator, int port, int lastPort)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test:default -t 2000 -p " + port;
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        String lastRef = "test:default -t 2000 -p " + lastPort;
        Ice.ObjectPrx lastBase = communicator.stringToProxy(lastRef);
        test(lastBase != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestPrx obj = TestPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        TestPrx lastObj = TestPrxHelper.checkedCast(lastBase);
        test(lastObj != null);
        test(lastObj.equals(lastBase));
        System.out.println("ok");

        System.out.print("shutting down all servers with single call... ");
        System.out.flush();
        obj.shutdown();
        System.out.println("ok");

        System.out.print("testing whether all servers are gone... ");
        System.out.flush();
        try
        {
            lastObj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
        }
    }
}
