// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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

    public static GPrx
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "d:default -p 12345 -t 2000";
        Ice.ObjectPrx db = communicator.stringToProxy(ref);
        test(db != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        DPrx d = DPrxHelper.checkedCast(db);
        test(d != null);
        test(d.equals(db));
        System.out.println("ok");

        System.out.print("testing non-facets A, B, C, and D... ");
        System.out.flush();
        test(d.callA().equals("A"));
        test(d.callB().equals("B"));
        test(d.callC().equals("C"));
        test(d.callD().equals("D"));
        System.out.println("ok");

        System.out.print("testing facets A, B, C, and D... ");
        System.out.flush();
        DPrx df = DPrxHelper.checkedCast(d, "facetABCD");
        test(df != null);
        test(df.callA().equals("A"));
        test(df.callB().equals("B"));
        test(df.callC().equals("C"));
        test(df.callD().equals("D"));
        System.out.println("ok");

        System.out.print("testing facets E and F... ");
        System.out.flush();
        FPrx ff = FPrxHelper.checkedCast(d, "facetEF");
        test(ff != null);
        test(ff.callE().equals("E"));
        test(ff.callF().equals("F"));
        System.out.println("ok");

        System.out.print("testing facet G... ");
        System.out.flush();
        GPrx gf = GPrxHelper.checkedCast(d, "facetG");
        test(gf != null);
        test(gf.callG().equals("G"));
        System.out.println("ok");

        return gf;
    }
}
