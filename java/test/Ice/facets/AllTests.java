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
	boolean gotException = false;
	try
	{
            adapter.addFacet(obj, Ice.Util.stringToIdentity("d"), "facetABCD");
	}
	catch(Ice.AlreadyRegisteredException ex)
	{
	    gotException = true;
	}
	test(gotException);
	adapter.removeFacet(Ice.Util.stringToIdentity("d"), "facetABCD");
	gotException = false;
	try
	{
            adapter.removeFacet(Ice.Util.stringToIdentity("d"), "facetABCD");
	}
	catch(Ice.NotRegisteredException ex)
	{
	    gotException = true;
	}
	test(gotException);
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
