//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

import java.io.PrintWriter;

import test.Ice.facets.Test.DPrx;
import test.Ice.facets.Test.FPrx;
import test.Ice.facets.Test.GPrx;
import test.Ice.facets.Test.HPrx;

import com.zeroc.Ice.ObjectPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static GPrx allTests(test.TestHelper helper)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing Ice.Admin.Facets property... ");
        test(communicator.getProperties().getPropertyAsList("Ice.Admin.Facets").length == 0);
        communicator.getProperties().setProperty("Ice.Admin.Facets", "foobar");
        String[] facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
        test(facetFilter.length == 1 && facetFilter[0].equals("foobar"));
        communicator.getProperties().setProperty("Ice.Admin.Facets", "foo\\'bar");
        facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
        test(facetFilter.length == 1 && facetFilter[0].equals("foo'bar"));
        communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
        facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
        test(facetFilter.length == 3 && facetFilter[0].equals("foo bar") && facetFilter[1].equals("toto")
             && facetFilter[2].equals("titi"));
        communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
        facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
        test(facetFilter.length == 2 && facetFilter[0].equals("foo bar' toto") && facetFilter[1].equals("titi"));
        // communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
        // facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
        // test(facetFilter.length == 0);
        communicator.getProperties().setProperty("Ice.Admin.Facets", "");
        out.println("ok");

        out.print("testing facet registration exceptions... ");
        final String host = communicator.getProperties().getPropertyAsInt("Ice.IPv6") != 0 ? "::1" : "127.0.0.1";
        communicator.getProperties().setProperty("FacetExceptionTestAdapter.Endpoints", "tcp -h \"" + host + "\"");
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter");
        com.zeroc.Ice.Object obj = new EmptyI();
        adapter.add(obj, com.zeroc.Ice.Util.stringToIdentity("d"));
        adapter.addFacet(obj, com.zeroc.Ice.Util.stringToIdentity("d"), "facetABCD");
        try
        {
            adapter.addFacet(obj, com.zeroc.Ice.Util.stringToIdentity("d"), "facetABCD");
            test(false);
        }
        catch(com.zeroc.Ice.AlreadyRegisteredException ex)
        {
        }
        adapter.removeFacet(com.zeroc.Ice.Util.stringToIdentity("d"), "facetABCD");
        try
        {
            adapter.removeFacet(com.zeroc.Ice.Util.stringToIdentity("d"), "facetABCD");
            test(false);
        }
        catch(com.zeroc.Ice.NotRegisteredException ex)
        {
        }
        out.println("ok");

        out.print("testing removeAllFacets... ");
        com.zeroc.Ice.Object obj1 = new EmptyI();
        com.zeroc.Ice.Object obj2 = new EmptyI();
        adapter.addFacet(obj1, com.zeroc.Ice.Util.stringToIdentity("id1"), "f1");
        adapter.addFacet(obj2, com.zeroc.Ice.Util.stringToIdentity("id1"), "f2");
        com.zeroc.Ice.Object obj3 = new EmptyI();
        adapter.addFacet(obj1, com.zeroc.Ice.Util.stringToIdentity("id2"), "f1");
        adapter.addFacet(obj2, com.zeroc.Ice.Util.stringToIdentity("id2"), "f2");
        adapter.addFacet(obj3, com.zeroc.Ice.Util.stringToIdentity("id2"), "");
        java.util.Map<String, com.zeroc.Ice.Object> fm =
            adapter.removeAllFacets(com.zeroc.Ice.Util.stringToIdentity("id1"));
        test(fm.size() == 2);
        test(fm.get("f1") == obj1);
        test(fm.get("f2") == obj2);
        try
        {
            adapter.removeAllFacets(com.zeroc.Ice.Util.stringToIdentity("id1"));
            test(false);
        }
        catch(com.zeroc.Ice.NotRegisteredException ex)
        {
        }
        fm = adapter.removeAllFacets(com.zeroc.Ice.Util.stringToIdentity("id2"));
        test(fm.size() == 3);
        test(fm.get("f1") == obj1);
        test(fm.get("f2") == obj2);
        test(fm.get("") == obj3);
        out.println("ok");

        adapter.deactivate();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "d:" + helper.getTestEndpoint(0);
        ObjectPrx db = communicator.stringToProxy(ref);
        test(db != null);
        out.println("ok");

        out.print("testing unchecked cast... ");
        out.flush();
        ObjectPrx prx = ObjectPrx.uncheckedCast(db);
        test(prx.ice_getFacet().length() == 0);
        prx = ObjectPrx.uncheckedCast(db, "facetABCD");
        test(prx.ice_getFacet() == "facetABCD");
        ObjectPrx prx2 = ObjectPrx.uncheckedCast(prx);
        test(prx2.ice_getFacet() == "facetABCD");
        ObjectPrx prx3 = ObjectPrx.uncheckedCast(prx, "");
        test(prx3.ice_getFacet().length() == 0);
        DPrx d = DPrx.uncheckedCast(db);
        test(d.ice_getFacet().length() == 0);
        DPrx df = DPrx.uncheckedCast(db, "facetABCD");
        test(df.ice_getFacet() == "facetABCD");
        DPrx df2 = DPrx.uncheckedCast(df);
        test(df2.ice_getFacet() == "facetABCD");
        DPrx df3 = DPrx.uncheckedCast(df, "");
        test(df3.ice_getFacet().length() == 0);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        prx = ObjectPrx.checkedCast(db);
        test(prx.ice_getFacet().length() == 0);
        prx = ObjectPrx.checkedCast(db, "facetABCD");
        test(prx.ice_getFacet() == "facetABCD");
        prx2 = ObjectPrx.checkedCast(prx);
        test(prx2.ice_getFacet() == "facetABCD");
        prx3 = ObjectPrx.checkedCast(prx, "");
        test(prx3.ice_getFacet().length() == 0);
        d = DPrx.checkedCast(db);
        test(d.ice_getFacet().length() == 0);
        df = DPrx.checkedCast(db, "facetABCD");
        test(df.ice_getFacet() == "facetABCD");
        df2 = DPrx.checkedCast(df);
        test(df2.ice_getFacet() == "facetABCD");
        df3 = DPrx.checkedCast(df, "");
        test(df3.ice_getFacet().length() == 0);
        out.println("ok");

        out.print("testing non-facets A, B, C, and D... ");
        out.flush();
        d = DPrx.checkedCast(db);
        test(d != null);
        test(d.equals(db));
        test(d.callA().equals("A"));
        test(d.callB().equals("B"));
        test(d.callC().equals("C"));
        test(d.callD().equals("D"));
        out.println("ok");

        out.print("testing facets A, B, C, and D... ");
        out.flush();
        df = DPrx.checkedCast(d, "facetABCD");
        test(df != null);
        test(df.callA().equals("A"));
        test(df.callB().equals("B"));
        test(df.callC().equals("C"));
        test(df.callD().equals("D"));
        out.println("ok");

        out.print("testing facets E and F... ");
        out.flush();
        FPrx ff = FPrx.checkedCast(d, "facetEF");
        test(ff != null);
        test(ff.callE().equals("E"));
        test(ff.callF().equals("F"));
        out.println("ok");

        out.print("testing facet G... ");
        out.flush();
        GPrx gf = GPrx.checkedCast(ff, "facetGH");
        test(gf != null);
        test(gf.callG().equals("G"));
        out.println("ok");

        out.print("testing whether casting preserves the facet... ");
        out.flush();
        HPrx hf = HPrx.checkedCast(gf);
        test(hf != null);
        test(hf.callG().equals("G"));
        test(hf.callH().equals("H"));
        out.println("ok");

        return gf;
    }
}
