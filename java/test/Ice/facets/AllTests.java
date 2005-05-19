// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    public static GPrx
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing facet registration exceptions... ");
	Ice.ObjectAdapter adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter");
	Ice.Object obj = new EmptyI();
        adapter.add(obj, Ice.Util.stringToIdentity("d"));
	adapter.addFacet(obj, Ice.Util.stringToIdentity("d"), "facetABCD");
	try
	{
            adapter.addFacet(obj, Ice.Util.stringToIdentity("d"), "facetABCD");
	    test(false);
	}
	catch(Ice.AlreadyRegisteredException ex)
	{
	}
	adapter.removeFacet(Ice.Util.stringToIdentity("d"), "facetABCD");
	try
	{
            adapter.removeFacet(Ice.Util.stringToIdentity("d"), "facetABCD");
	    test(false);
	}
	catch(Ice.NotRegisteredException ex)
	{
	}
        System.out.println("ok");

        System.out.print("testing removeAllFacets... ");
	Ice.Object obj1 = new EmptyI();
	Ice.Object obj2 = new EmptyI();
	adapter.addFacet(obj1, Ice.Util.stringToIdentity("id1"), "f1");
	adapter.addFacet(obj2, Ice.Util.stringToIdentity("id1"), "f2");
	Ice.Object obj3 = new EmptyI();
	adapter.addFacet(obj1, Ice.Util.stringToIdentity("id2"), "f1");
	adapter.addFacet(obj2, Ice.Util.stringToIdentity("id2"), "f2");
	adapter.addFacet(obj3, Ice.Util.stringToIdentity("id2"), "");
	java.util.Map fm = adapter.removeAllFacets(Ice.Util.stringToIdentity("id1"));
	test(fm.size() == 2);
	test(fm.get("f1") == obj1);
	test(fm.get("f2") == obj2);
	try
	{
            adapter.removeAllFacets(Ice.Util.stringToIdentity("id1"));
	    test(false);
	}
	catch(Ice.NotRegisteredException ex)
	{
	}
	fm = adapter.removeAllFacets(Ice.Util.stringToIdentity("id2"));
	test(fm.size() == 3);
	test(fm.get("f1") == obj1);
	test(fm.get("f2") == obj2);
	test(fm.get("") == obj3);
        System.out.println("ok");

        adapter.deactivate();

        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "d:default -p 12345 -t 10000";
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
        GPrx gf = GPrxHelper.checkedCast(ff, "facetGH");
        test(gf != null);
        test(gf.callG().equals("G"));
        System.out.println("ok");

        System.out.print("testing whether casting preserves the facet... ");
        System.out.flush();
        HPrx hf = HPrxHelper.checkedCast(gf);
        test(hf != null);
        test(hf.callG().equals("G"));
        test(hf.callH().equals("H"));
        System.out.println("ok");

        return gf;
    }
}
