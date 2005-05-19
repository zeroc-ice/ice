// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
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
        Console.Write("testing facet registration exceptions... ");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter");
        Ice.Object obj = new EmptyI();
        adapter.add(obj, Ice.Util.stringToIdentity("d"));
        adapter.addFacet(obj, Ice.Util.stringToIdentity("d"), "facetABCD");
        try
        {
            adapter.addFacet(obj, Ice.Util.stringToIdentity("d"), "facetABCD");
	    test(false);
        }
        catch(Ice.AlreadyRegisteredException)
        {
        }
        adapter.removeFacet(Ice.Util.stringToIdentity("d"), "facetABCD");
        try
        {
            adapter.removeFacet(Ice.Util.stringToIdentity("d"), "facetABCD");
	    test(false);
        }
        catch(Ice.NotRegisteredException)
        {
        }
        Console.WriteLine("ok");

        Console.Write("testing removeAllFacets... ");
	Ice.Object obj1 = new EmptyI();
	Ice.Object obj2 = new EmptyI();
	adapter.addFacet(obj1, Ice.Util.stringToIdentity("id1"), "f1");
	adapter.addFacet(obj2, Ice.Util.stringToIdentity("id1"), "f2");
	Ice.Object obj3 = new EmptyI();
	adapter.addFacet(obj1, Ice.Util.stringToIdentity("id2"), "f1");
	adapter.addFacet(obj2, Ice.Util.stringToIdentity("id2"), "f2");
	adapter.addFacet(obj3, Ice.Util.stringToIdentity("id2"), "");
	Ice.FacetMap fm = adapter.removeAllFacets(Ice.Util.stringToIdentity("id1"));
	test(fm.Count == 2);
	test(fm["f1"] == obj1);
	test(fm["f2"] == obj2);
	try
	{
            adapter.removeAllFacets(Ice.Util.stringToIdentity("id1"));
	    test(false);
	}
	catch(Ice.NotRegisteredException)
	{
	}
	fm = adapter.removeAllFacets(Ice.Util.stringToIdentity("id2"));
	test(fm.Count == 3);
	test(fm["f1"] == obj1);
	test(fm["f2"] == obj2);
	test(fm[""] == obj3);
        Console.WriteLine("ok");

        adapter.deactivate();
        
        Console.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string @ref = "d:default -p 12345 -t 2000";
        Ice.ObjectPrx db = communicator.stringToProxy(@ref);
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
