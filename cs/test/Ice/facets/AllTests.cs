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
            throw new System.Exception();
        }
    }
    
    public static GPrx allTests(Ice.Communicator communicator)
    {
        Console.Write("testing facet registration exceptions... ");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter");
        Ice.Object obj = new EmptyI();
        adapter.add(obj, Ice.Util.stringToIdentity("d"));
        obj.ice_addFacet(obj, "facetABCD");
        bool gotException = false;
        try
        {
            obj.ice_addFacet(obj, "facetABCD");
        }
        catch(Ice.AlreadyRegisteredException)
        {
            gotException = true;
        }
        test(gotException);
        obj.ice_removeFacet("facetABCD");
        gotException = false;
        try
        {
            obj.ice_removeFacet("facetABCD");
        }
        catch(Ice.NotRegisteredException)
        {
            gotException = true;
        }
        test(gotException);
        Console.WriteLine("ok");
        
        Console.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string ref_Renamed = "d:default -p 12345 -t 2000";
        Ice.ObjectPrx db = communicator.stringToProxy(ref_Renamed);
        test(db != null);
        Console.WriteLine("ok");
        
        Console.Write("testing checked cast... ");
        Console.Out.Flush();
        DPrx d = DPrxHelper.checkedCast(db);
        test(d != null);
        test(d.Equals(db));
        Console.WriteLine("ok");
        
        Console.Write("testing non-facets A, B, C, and D... ");
        Console.Out.Flush();
        test(d.callA().Equals("A"));
        test(d.callB().Equals("B"));
        test(d.callC().Equals("C"));
        test(d.callD().Equals("D"));
        Console.WriteLine("ok");
        
        Console.Write("testing facets A, B, C, and D... ");
        Console.Out.Flush();
        DPrx df = DPrxHelper.checkedCast(d, "facetABCD");
        test(df != null);
        test(df.callA().Equals("A"));
        test(df.callB().Equals("B"));
        test(df.callC().Equals("C"));
        test(df.callD().Equals("D"));
        Console.WriteLine("ok");
        
        Console.Write("testing facets E and F... ");
        Console.Out.Flush();
        FPrx ff = FPrxHelper.checkedCast(d, "facetEF");
        test(ff != null);
        test(ff.callE().Equals("E"));
        test(ff.callF().Equals("F"));
        Console.WriteLine("ok");
        
        Console.Write("testing facet G, which is a sub-facet of E and F... ");
        Console.Out.Flush();
        GPrx gf = GPrxHelper.checkedCast(ff, "facetGH");
        test(gf != null);
        test(gf.callG().Equals("G"));
        Console.WriteLine("ok");
        
        Console.Write("testing whether casting preserves the facet... ");
        Console.Out.Flush();
        HPrx hf = HPrxHelper.checkedCast(gf);
        test(hf != null);
        test(hf.callG().Equals("G"));
        test(hf.callH().Equals("H"));
        Console.WriteLine("ok");
        
        return gf;
    }
}
