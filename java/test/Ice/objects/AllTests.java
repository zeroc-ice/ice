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
        if (!b)
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
        test(b1.b == b1);
        test(b1.c == null);
        test(b1.a instanceof B);
        test(((B)b1.a).a == b1.a);
        test(((B)b1.a).b == b1);
        test(((B)b1.a).c instanceof C);
        test(((C)(((B)b1.a).c)).b == b1.a);
        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        test(b2.a == b2);
        test(d.c == null);
        System.out.println("ok");

        //
        // Break cyclic dependencies
        //
        if (!collocated)
        {
            ((B)b1.a).a = null;
            ((B)b1.a).b = null;
            ((B)b1.a).c = null;
            b1.a = null;
            b1.b = null;

            b2.a = null;
            b2.b.a = null;
            b2.b.b = null;
            b2.c = null;

            c.b.a = null;
            c.b.b.a = null;
            c.b.b.b = null;
            c.b = null;

            ((B)((B)d.a).a).a = null;
            ((B)((B)d.a).a).b = null;
            ((B)d.a).b.a = null;
            ((B)d.a).b.b = null;
            d.b.a = null;
            d.b.b = null;
            d.b.c = null;
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
        test(b1.a == b2);
        test(b1.b == b1);
        test(b1.c == null);
        test(b2.a == b2);
        test(b2.b == b1);
        test(b2.c == c);
        test(c.b == b2);
        test(d.a == b1);
        test(d.b == b2);
        test(d.c == null);
        System.out.println("ok");

        //
        // Break cyclic dependencies
        //
        if (!collocated)
        {
            b1.a = null;
            b1.b = null;
            b2.a = null;
            b2.b = null;
            b2.c = null;
            c.b = null;
            d.a = null;
            d.b = null;
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
        test(b1.b == b1);
        test(b1.c == null);
        test(b1.a instanceof B);
        test(((B)b1.a).a == b1.a);
        test(((B)b1.a).b == b1);
        test(((B)b1.a).c instanceof C);
        test(((C)((B)b1.a).c).b == b1.a);
        System.out.println("ok");

        System.out.print("checking facet consistency... ");
        System.out.flush();
        B fb1 = (B)b1.ice_findFacet("b1");
        B fb2 = (B)b1.ice_findFacet("b2");
        C fc = (C)b1.ice_findFacet("c");
        D fd = (D)b1.ice_findFacet("d");
        test(b1 == fb1);
        test(fb1.a == fb2);
        test(fb1.b == fb1);
        test(fb1.c == null);
        test(fb2.a == fb2);
        test(fb2.b == fb1);
        test(fb2.c == fc);
        test(fc.b == fb2);
        test(fd.a == fb1);
        test(fd.b == fb2);
        test(fd.c == null);
        System.out.println("ok");

        //
        // Break cyclic dependencies
        //
        if (!collocated)
        {
            ((B)fb1.a).a = null;
            ((B)fb1.a).b = null;
            fb1.a = null;
            fb1.b = null;
            fb1.ice_removeAllFacets();
            fb2.a = null;
            fb2.b = null;
            fb2.c = null;
            fc.b = null;
            fd.a = null;
            fd.b = null;
        }

        System.out.print("getting B1 with facets, and B2, C, and D all at " +
                         "once... ");
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
        test(b1.a == b2);
        test(b1.b == b1);
        test(b1.c == null);
        test(b2.a == b2);
        test(b2.b == b1);
        test(b2.c == c);
        test(c.b == b2);
        test(d.a == b1);
        test(d.b == b2);
        test(d.c == null);
        System.out.println("ok");
            
        System.out.print("checking facet consistency... ");
        System.out.flush();
        fb1 = (B)b1.ice_findFacet("b1");
        fb2 = (B)b1.ice_findFacet("b2");
        fc = (C)b1.ice_findFacet("c");
        fd = (D)b1.ice_findFacet("d");
        test(b1 == fb1);
        test(b2 == fb2);
        test(c == fc);
        test(d == fd);
        test(fb1.a == fb2);
        test(fb1.b == fb1);
        test(fb1.c == null);
        test(fb2.a == fb2);
        test(fb2.b == fb1);
        test(fb2.c == fc);
        test(fc.b == fb2);
        test(fd.a == fb1);
        test(fd.b == fb2);
        test(fd.c == null);
        System.out.println("ok");

        //
        // Break cyclic dependencies
        //
        if (!collocated)
        {
            fb1.a = null;
            fb1.b = null;
            fb1.ice_removeAllFacets();
            fb2.a = null;
            fb2.b = null;
            fb2.c = null;
            fc.b = null;
            fd.a = null;
            fd.b = null;
        }

        return initial;
    }
}
