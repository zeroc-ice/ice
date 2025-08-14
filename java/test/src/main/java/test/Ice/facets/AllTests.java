// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.AlreadyRegisteredException;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.NotRegisteredException;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Util;

import test.Ice.facets.Test.DPrx;
import test.Ice.facets.Test.FPrx;
import test.Ice.facets.Test.GPrx;
import test.Ice.facets.Test.HPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.Map;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static GPrx allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing Ice.Admin.Facets property... ");
        test(communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets").length == 0);
        communicator.getProperties().setProperty("Ice.Admin.Facets", "foobar");
        String[] facetFilter =
            communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
        test(facetFilter.length == 1 && "foobar".equals(facetFilter[0]));
        communicator.getProperties().setProperty("Ice.Admin.Facets", "foo\\'bar");
        facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
        test(facetFilter.length == 1 && "foo'bar".equals(facetFilter[0]));
        communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
        facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
        test(
            facetFilter.length == 3
                && "foo bar".equals(facetFilter[0])
                && "toto".equals(facetFilter[1])
                && "titi".equals(facetFilter[2]));
        communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
        facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
        test(
            facetFilter.length == 2
                && "foo bar' toto".equals(facetFilter[0])
                && "titi".equals(facetFilter[1]));
        // communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
        // facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
        // test(facetFilter.length == 0);
        communicator.getProperties().setProperty("Ice.Admin.Facets", "");
        out.println("ok");

        out.print("testing add facet with uuid... ");
        {
            var testAdapter = communicator.createObjectAdapterWithEndpoints("TestAdapter2", "default");
            test("facetABCD".equals(testAdapter.addFacetWithUUID(new EmptyI(), "facetABCD").ice_getFacet()));
            testAdapter.destroy();
        }
        out.println("ok");

        out.print("testing facet registration exceptions... ");
        final String host =
            communicator.getProperties().getIcePropertyAsInt("Ice.IPv6") != 0
                ? "::1"
                : "127.0.0.1";
        communicator
            .getProperties()
            .setProperty("FacetExceptionTestAdapter.Endpoints", "tcp -h \"" + host + "\"");
        ObjectAdapter adapter =
            communicator.createObjectAdapter("FacetExceptionTestAdapter");
        Object obj = new EmptyI();
        adapter.add(obj, Util.stringToIdentity("d"));
        adapter.addFacet(obj, Util.stringToIdentity("d"), "facetABCD");
        try {
            adapter.addFacet(obj, Util.stringToIdentity("d"), "facetABCD");
            test(false);
        } catch (AlreadyRegisteredException ex) {}
        adapter.removeFacet(Util.stringToIdentity("d"), "facetABCD");
        try {
            adapter.removeFacet(Util.stringToIdentity("d"), "facetABCD");
            test(false);
        } catch (NotRegisteredException ex) {}
        out.println("ok");

        out.print("testing removeAllFacets... ");
        Object obj1 = new EmptyI();
        Object obj2 = new EmptyI();
        adapter.addFacet(obj1, Util.stringToIdentity("id1"), "f1");
        adapter.addFacet(obj2, Util.stringToIdentity("id1"), "f2");
        Object obj3 = new EmptyI();
        adapter.addFacet(obj1, Util.stringToIdentity("id2"), "f1");
        adapter.addFacet(obj2, Util.stringToIdentity("id2"), "f2");
        adapter.addFacet(obj3, Util.stringToIdentity("id2"), "");
        Map<String, Object> fm =
            adapter.removeAllFacets(Util.stringToIdentity("id1"));
        test(fm.size() == 2);
        test(fm.get("f1") == obj1);
        test(fm.get("f2") == obj2);
        try {
            adapter.removeAllFacets(Util.stringToIdentity("id1"));
            test(false);
        } catch (NotRegisteredException ex) {}
        fm = adapter.removeAllFacets(Util.stringToIdentity("id2"));
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
        test(prx.ice_getFacet().isEmpty());
        prx = ObjectPrx.uncheckedCast(db, "facetABCD");
        test(prx.ice_getFacet() == "facetABCD");
        ObjectPrx prx2 = ObjectPrx.uncheckedCast(prx);
        test(prx2.ice_getFacet() == "facetABCD");
        ObjectPrx prx3 = ObjectPrx.uncheckedCast(prx, "");
        test(prx3.ice_getFacet().isEmpty());
        DPrx d = DPrx.uncheckedCast(db);
        test(d.ice_getFacet().isEmpty());
        DPrx df = DPrx.uncheckedCast(db, "facetABCD");
        test(df.ice_getFacet() == "facetABCD");
        DPrx df2 = DPrx.uncheckedCast(df);
        test(df2.ice_getFacet() == "facetABCD");
        DPrx df3 = DPrx.uncheckedCast(df, "");
        test(df3.ice_getFacet().isEmpty());
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        prx = ObjectPrx.checkedCast(db);
        test(prx.ice_getFacet().isEmpty());
        prx = ObjectPrx.checkedCast(db, "facetABCD");
        test(prx.ice_getFacet() == "facetABCD");
        prx2 = ObjectPrx.checkedCast(prx);
        test(prx2.ice_getFacet() == "facetABCD");
        prx3 = ObjectPrx.checkedCast(prx, "");
        test(prx3.ice_getFacet().isEmpty());
        d = DPrx.checkedCast(db);
        test(d.ice_getFacet().isEmpty());
        df = DPrx.checkedCast(db, "facetABCD");
        test(df.ice_getFacet() == "facetABCD");
        df2 = DPrx.checkedCast(df);
        test(df2.ice_getFacet() == "facetABCD");
        df3 = DPrx.checkedCast(df, "");
        test(df3.ice_getFacet().isEmpty());
        out.println("ok");

        out.print("testing non-facets A, B, C, and D... ");
        out.flush();
        d = DPrx.checkedCast(db);
        test(d != null);
        test(d.equals(db));
        test("A".equals(d.callA()));
        test("B".equals(d.callB()));
        test("C".equals(d.callC()));
        test("D".equals(d.callD()));
        out.println("ok");

        out.print("testing facets A, B, C, and D... ");
        out.flush();
        df = DPrx.checkedCast(d, "facetABCD");
        test(df != null);
        test("A".equals(df.callA()));
        test("B".equals(df.callB()));
        test("C".equals(df.callC()));
        test("D".equals(df.callD()));
        out.println("ok");

        out.print("testing facets E and F... ");
        out.flush();
        FPrx ff = FPrx.checkedCast(d, "facetEF");
        test(ff != null);
        test("E".equals(ff.callE()));
        test("F".equals(ff.callF()));
        out.println("ok");

        out.print("testing facet G... ");
        out.flush();
        GPrx gf = GPrx.checkedCast(ff, "facetGH");
        test(gf != null);
        test("G".equals(gf.callG()));
        out.println("ok");

        out.print("testing whether casting preserves the facet... ");
        out.flush();
        HPrx hf = HPrx.checkedCast(gf);
        test(hf != null);
        test("G".equals(hf.callG()));
        test("H".equals(hf.callH()));
        out.println("ok");

        return gf;
    }

    private AllTests() {}
}
