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

    public static InitialPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "initial:default -p 12345 -t 2000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        InitialPrx initial = InitialPrxHelper.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        System.out.println("ok");

        System.out.print("getting B1... ");
        System.out.flush();
        B b1 = initial.getB1();
        test(b1 != null);
        System.out.println("ok");

        System.out.print("getting B2... ");
        System.out.flush();
        B b2 = initial.getB2();
        test(b2 != null);
        System.out.println("ok");

        System.out.print("getting C... ");
        System.out.flush();
        C c = initial.getC();
        test(c != null);
        System.out.println("ok");

        System.out.print("getting D... ");
        System.out.flush();
        D d = initial.getD();
        test(d != null);
        System.out.println("ok");

        System.out.print("checking consistency... ");
        System.out.flush();
        test(b1 != b2);
        //test(b1 != c);
        //test(b1 != d);
        //test(b2 != c);
        //test(b2 != d);
        //test(c != d);
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b1.theA instanceof B);
        test(((B)b1.theA).theA == b1.theA);
        test(((B)b1.theA).theB == b1);
        test(((B)b1.theA).theC instanceof C);
        test(((C)(((B)b1.theA).theC)).theB == b1.theA);
        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        test(b2.theA == b2);
        test(d.theC == null);
        System.out.println("ok");

        //
        // Break cyclic dependencies
        //
        if(!collocated)
        {
            ((B)b1.theA).theA = null;
            ((B)b1.theA).theB = null;
            ((B)b1.theA).theC = null;
            b1.theA = null;
            b1.theB = null;

            b2.theA = null;
            b2.theB.theA = null;
            b2.theB.theB = null;
            b2.theC = null;

            c.theB.theA = null;
            c.theB.theB.theA = null;
            c.theB.theB.theB = null;
            c.theB = null;

            ((B)((B)d.theA).theA).theA = null;
            ((B)((B)d.theA).theA).theB = null;
            ((B)d.theA).theB.theA = null;
            ((B)d.theA).theB.theB = null;
            d.theB.theA = null;
            d.theB.theB = null;
            d.theB.theC = null;
        }

        System.out.print("getting B1, B2, C, and D all at once... ");
        System.out.flush();
        BHolder b1H = new BHolder();
        BHolder b2H = new BHolder();
        CHolder cH = new CHolder();
        DHolder dH = new DHolder();
        initial.getAll(b1H, b2H, cH, dH);
        b1 = b1H.value;
        b2 = b2H.value;
        c = cH.value;
        d = dH.value;
        test(b1 != null);
        test(b2 != null);
        test(c != null);
        test(d != null);
        System.out.println("ok");

        System.out.print("checking consistency... ");
        System.out.flush();
        test(b1 != b2);
        //test(b1 != c);
        //test(b1 != d);
        //test(b2 != c);
        //test(b2 != d);
        //test(c != d);
        test(b1.theA == b2);
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b2.theA == b2);
        test(b2.theB == b1);
        test(b2.theC == c);
        test(c.theB == b2);
        test(d.theA == b1);
        test(d.theB == b2);
        test(d.theC == null);
        System.out.println("ok");

        //
        // Break cyclic dependencies
        //
        if(!collocated)
        {
            b1.theA = null;
            b1.theB = null;
            b2.theA = null;
            b2.theB = null;
            b2.theC = null;
            c.theB = null;
            d.theA = null;
            d.theB = null;
        }

        System.out.print("adding facets to B1... ");
        System.out.flush();
        initial.addFacetsToB1();
        System.out.println("ok");

        System.out.print("getting B1 with facets... ");
        System.out.flush();
        b1 = initial.getB1();
        test(b1 != null);
        System.out.println("ok");

        System.out.print("checking consistency... ");
        System.out.flush();
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b1.theA instanceof B);
        test(((B)b1.theA).theA == b1.theA);
        test(((B)b1.theA).theB == b1);
        test(((B)b1.theA).theC instanceof C);
        test(((C)((B)b1.theA).theC).theB == b1.theA);
        System.out.println("ok");

        System.out.print("checking facet consistency... ");
        System.out.flush();
        B fb1 = (B)b1.ice_findFacet("b1");
        B fb2 = (B)b1.ice_findFacet("b2");
        C fc = (C)fb2.ice_findFacet("c");
        D fd = (D)fb2.ice_findFacet("d");
        test(b1 == fb1);
        test(fb1.theA == fb2);
        test(fb1.theB == fb1);
        test(fb1.theC == null);
        test(fb2.theA == fb2);
        test(fb2.theB == fb1);
        test(fb2.theC == fc);
        test(fc.theB == fb2);
        test(fd.theA == fb1);
        test(fd.theB == fb2);
        test(fd.theC == null);
        System.out.println("ok");

        //
        // Break cyclic dependencies
        //
        if(!collocated)
        {
            ((B)fb1.theA).theA = null;
            ((B)fb1.theA).theB = null;
            fb1.theA = null;
            fb1.theB = null;
            fb1.ice_removeAllFacets();
            fb2.theA = null;
            fb2.theB = null;
            fb2.theC = null;
            fc.theB = null;
            fd.theA = null;
            fd.theB = null;
        }

        System.out.print("getting B1 with facets, and B2, C, and D all at once... ");
        System.out.flush();
        initial.getAll(b1H, b2H, cH, dH);
        b1 = b1H.value;
        b2 = b2H.value;
        c = cH.value;
        d = dH.value;
        test(b1 != null);
        test(b2 != null);
        test(c != null);
        test(d != null);
        System.out.println("ok");

        System.out.print("checking consistency... ");
        System.out.flush();
        test(b1 != b2);
        //test(b1 != c);
        //test(b1 != d);
        //test(b2 != c);
        //test(b2 != d);
        //test(c != d);
        test(b1.theA == b2);
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b2.theA == b2);
        test(b2.theB == b1);
        test(b2.theC == c);
        test(c.theB == b2);
        test(d.theA == b1);
        test(d.theB == b2);
        test(d.theC == null);
        System.out.println("ok");
            
        System.out.print("checking facet consistency... ");
        System.out.flush();
        fb1 = (B)b1.ice_findFacet("b1");
        fb2 = (B)b1.ice_findFacet("b2");
        fc = (C)fb2.ice_findFacet("c");
        fd = (D)fb2.ice_findFacet("d");
        test(b1 == fb1);
        test(b2 == fb2);
        test(c == fc);
        test(d == fd);
        test(fb1.theA == fb2);
        test(fb1.theB == fb1);
        test(fb1.theC == null);
        test(fb2.theA == fb2);
        test(fb2.theB == fb1);
        test(fb2.theC == fc);
        test(fc.theB == fb2);
        test(fd.theA == fb1);
        test(fd.theB == fb2);
        test(fd.theC == null);
        System.out.println("ok");

        //
        // Break cyclic dependencies
        //
        if(!collocated)
        {
            fb1.theA = null;
            fb1.theB = null;
            fb1.ice_removeAllFacets();
            fb2.theA = null;
            fb2.theB = null;
            fb2.theC = null;
            fc.theB = null;
            fd.theA = null;
            fd.theB = null;
        }

        return initial;
    }
}
