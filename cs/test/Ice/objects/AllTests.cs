// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


using System;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }
    
    public static InitialPrx allTests(Ice.Communicator communicator, bool collocated)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        String ref_Renamed = "initial:default -p 12345 -t 2000";
        Ice.ObjectPrx base_Renamed = communicator.stringToProxy(ref_Renamed);
        test(base_Renamed != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        InitialPrx initial = InitialPrxHelper.checkedCast(base_Renamed);
        test(initial != null);
        test(initial.Equals(base_Renamed));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting B1... ");
        Console.Out.Flush();
        B b1 = initial.getB1();
        test(b1 != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting B2... ");
        Console.Out.Flush();
        B b2 = initial.getB2();
        test(b2 != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting C... ");
        Console.Out.Flush();
        C c = initial.getC();
        test(c != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting D... ");
        Console.Out.Flush();
        D d = initial.getD();
        test(d != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("checking consistency... ");
        Console.Out.Flush();
        test(b1 != b2);
        //test(b1 != c);
        //test(b1 != d);
        //test(b2 != c);
        //test(b2 != d);
        //test(c != d);
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b1.theA is B);
        test(((B) b1.theA).theA == b1.theA);
        test(((B) b1.theA).theB == b1);
        test(((C) (((B) b1.theA).theC)).theB == b1.theA);
        // More tests possible for b2 and d, but I think this is already
        // sufficient.
        test(b2.theA == b2);
        test(d.theC == null);
        Console.Out.WriteLine("ok");
        
        //
        // Break cyclic dependencies
        //
        if(!collocated)
        {
            ((B) b1.theA).theA = null;
            ((B) b1.theA).theB = null;
            ((B) b1.theA).theC = null;
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
            
            ((B) ((B) d.theA).theA).theA = null;
            ((B) ((B) d.theA).theA).theB = null;
            ((B) d.theA).theB.theA = null;
            ((B) d.theA).theB.theB = null;
            d.theB.theA = null;
            d.theB.theB = null;
            d.theB.theC = null;
        }
        
        Console.Out.Write("getting B1, B2, C, and D all at once... ");
        Console.Out.Flush();
        B b1out;
        B b2out;
        C cout;
        D dout;
        initial.getAll(out b1out, out b2out, out cout, out dout);
        test(b1out != null);
        test(b2out != null);
        test(cout != null);
        test(dout != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("checking consistency... ");
        Console.Out.Flush();
        test(b1out != b2out);
        test(b1out.theA == b2out);
        test(b1out.theB == b1out);
        test(b1out.theC == null);
        test(b2out.theA == b2out);
        test(b2out.theB == b1out);
        test(b2out.theC == cout);
        test(cout.theB == b2out);
        test(dout.theA == b1out);
        test(dout.theB == b2out);
        test(dout.theC == null);
        Console.Out.WriteLine("ok");
        
        //
        // Break cyclic dependencies
        //
        if(!collocated)
        {
            b1out.theA = null;
            b1out.theB = null;
            b2out.theA = null;
            b2out.theB = null;
            b2out.theC = null;
            cout.theB = null;
            dout.theA = null;
            dout.theB = null;
        }
        
        Console.Out.Write("adding facets to B1... ");
        Console.Out.Flush();
        initial.addFacetsToB1();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("getting B1 with facets... ");
        Console.Out.Flush();
        b1 = initial.getB1();
        test(b1 != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("checking consistency... ");
        Console.Out.Flush();
        test(b1.theB == b1);
        test(b1.theC == null);
        test(b1.theA is B);
        test(((B) b1.theA).theA == b1.theA);
        test(((B) b1.theA).theB == b1);
        test(((C) ((B) b1.theA).theC).theB == b1.theA);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("checking facet consistency... ");
        Console.Out.Flush();
        B fb1 = (B) b1.ice_findFacet("b1");
        B fb2 = (B) b1.ice_findFacet("b2");
        C fc = (C) fb2.ice_findFacet("c");
        D fd = (D) fb2.ice_findFacet("d");
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
        Console.Out.WriteLine("ok");
        
        //
        // Break cyclic dependencies
        //
        if(!collocated)
        {
            ((B) fb1.theA).theA = null;
            ((B) fb1.theA).theB = null;
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
        
        Console.Out.Write("getting B1 with facets, and B2, C, and D all at once... ");
        Console.Out.Flush();
        initial.getAll(out b1out, out b2out, out cout, out dout);
        test(b1out != null);
        test(b2out != null);
        test(cout != null);
        test(dout != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("checking consistency... ");
        Console.Out.Flush();
        test(b1out != b2out);
        test(b1out.theA == b2out);
        test(b1out.theB == b1out);
        test(b1out.theC == null);
        test(b2out.theA == b2out);
        test(b2out.theB == b1out);
        test(b2out.theC == cout);
        test(cout.theB == b2out);
        test(dout.theA == b1out);
        test(dout.theB == b2out);
        test(dout.theC == null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("checking facet consistency... ");
        Console.Out.Flush();
        fb1 = (B)b1out.ice_findFacet("b1");
        fb2 = (B)b1out.ice_findFacet("b2");
        fc = (C)fb2.ice_findFacet("c");
        fd = (D)fb2.ice_findFacet("d");
        test(b1out == fb1);
        test(b2out == fb2);
        test(cout == fc);
        test(dout == fd);
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
        Console.Out.WriteLine("ok");
        
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
