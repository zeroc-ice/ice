// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Test;

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
        Console.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string @ref = "d:default -p 12010 -t 2000";
        Ice.ObjectPrx db = communicator.stringToProxy(@ref);
        test(db != null);
        Console.WriteLine("ok");

        Console.Write("testing unchecked cast... ");
        Console.Out.Flush();
        Ice.ObjectPrx prx = Ice.ObjectPrxHelper.uncheckedCast(db);
        test(prx.ice_getFacet().Length == 0);
        prx = Ice.ObjectPrxHelper.uncheckedCast(db, "facetABCD");
        test(prx.ice_getFacet() == "facetABCD");
        Ice.ObjectPrx prx2 = Ice.ObjectPrxHelper.uncheckedCast(prx);
        test(prx2.ice_getFacet() == "facetABCD");
        Ice.ObjectPrx prx3 = Ice.ObjectPrxHelper.uncheckedCast(prx, "");
        test(prx3.ice_getFacet().Length == 0);
        DPrx d = Test.DPrxHelper.uncheckedCast(db);
        test(d.ice_getFacet().Length == 0);
        DPrx df = Test.DPrxHelper.uncheckedCast(db, "facetABCD");
        test(df.ice_getFacet() == "facetABCD");
        DPrx df2 = Test.DPrxHelper.uncheckedCast(df);
        test(df2.ice_getFacet() == "facetABCD");
        DPrx df3 = Test.DPrxHelper.uncheckedCast(df, "");
        test(df3.ice_getFacet().Length == 0);
        Console.WriteLine("ok");

        Console.Write("testing checked cast... ");
        Console.Out.Flush();
        prx = Ice.ObjectPrxHelper.checkedCast(db);
        test(prx.ice_getFacet().Length == 0);
        prx = Ice.ObjectPrxHelper.checkedCast(db, "facetABCD");
        test(prx.ice_getFacet() == "facetABCD");
        prx2 = Ice.ObjectPrxHelper.checkedCast(prx);
        test(prx2.ice_getFacet() == "facetABCD");
        prx3 = Ice.ObjectPrxHelper.checkedCast(prx, "");
        test(prx3.ice_getFacet().Length == 0);
        d = Test.DPrxHelper.checkedCast(db);
        test(d.ice_getFacet().Length == 0);
        df = Test.DPrxHelper.checkedCast(db, "facetABCD");
        test(df.ice_getFacet() == "facetABCD");
        df2 = Test.DPrxHelper.checkedCast(df);
        test(df2.ice_getFacet() == "facetABCD");
        df3 = Test.DPrxHelper.checkedCast(df, "");
        test(df3.ice_getFacet().Length == 0);
        Console.WriteLine("ok");
        
        Console.Write("testing non-facets A, B, C, and D... ");
        Console.Out.Flush();
        d = DPrxHelper.checkedCast(db);
        test(d != null);
        test(d.Equals(db));
        test(d.callA().Equals("A"));
        test(d.callB().Equals("B"));
        test(d.callC().Equals("C"));
        test(d.callD().Equals("D"));
        Console.WriteLine("ok");
        
        Console.Write("testing facets A, B, C, and D... ");
        Console.Out.Flush();
        df = DPrxHelper.checkedCast(d, "facetABCD");
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
        
        Console.Write("testing facet G... ");
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
