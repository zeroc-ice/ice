//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    namespace facets
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.GPrx allTests(global::Test.TestHelper helper)
            {

                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing Ice.Admin.Facets property... ");
                test(communicator.getProperties().getPropertyAsList("Ice.Admin.Facets").Length == 0);
                communicator.getProperties().setProperty("Ice.Admin.Facets", "foobar");
                String[] facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
                test(facetFilter.Length == 1 && facetFilter[0].Equals("foobar"));
                communicator.getProperties().setProperty("Ice.Admin.Facets", "foo\\'bar");
                facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
                test(facetFilter.Length == 1 && facetFilter[0].Equals("foo'bar"));
                communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
                facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
                test(facetFilter.Length == 3 && facetFilter[0].Equals("foo bar") && facetFilter[1].Equals("toto")
                     && facetFilter[2].Equals("titi"));
                communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
                facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
                test(facetFilter.Length == 2 && facetFilter[0].Equals("foo bar' toto") && facetFilter[1].Equals("titi"));
                // communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
                // facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
                // test(facetFilter.Length == 0);
                communicator.getProperties().setProperty("Ice.Admin.Facets", "");
                output.WriteLine("ok");

                output.Write("testing facet registration exceptions... ");
                communicator.getProperties().setProperty("FacetExceptionTestAdapter.Endpoints", "tcp -h *");
                Ice.ObjectAdapter adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter");
                Ice.Object obj = new EmptyI();
                adapter.add(obj, Ice.Util.stringToIdentity("d"));
                adapter.addFacet(obj, Ice.Util.stringToIdentity("d"), "facetABCD");
                try
                {
                    adapter.addFacet(obj, Ice.Util.stringToIdentity("d"), "facetABCD");
                    test(false);
                }
                catch (Ice.AlreadyRegisteredException)
                {
                }
                adapter.removeFacet(Ice.Util.stringToIdentity("d"), "facetABCD");
                try
                {
                    adapter.removeFacet(Ice.Util.stringToIdentity("d"), "facetABCD");
                    test(false);
                }
                catch (Ice.NotRegisteredException)
                {
                }
                output.WriteLine("ok");

                output.Write("testing removeAllFacets... ");
                Ice.Object obj1 = new EmptyI();
                Ice.Object obj2 = new EmptyI();
                adapter.addFacet(obj1, Ice.Util.stringToIdentity("id1"), "f1");
                adapter.addFacet(obj2, Ice.Util.stringToIdentity("id1"), "f2");
                Ice.Object obj3 = new EmptyI();
                adapter.addFacet(obj1, Ice.Util.stringToIdentity("id2"), "f1");
                adapter.addFacet(obj2, Ice.Util.stringToIdentity("id2"), "f2");
                adapter.addFacet(obj3, Ice.Util.stringToIdentity("id2"), "");
                Dictionary<string, Ice.Object> fm
                    = adapter.removeAllFacets(Ice.Util.stringToIdentity("id1"));
                test(fm.Count == 2);
                test(fm["f1"] == obj1);
                test(fm["f2"] == obj2);
                try
                {
                    adapter.removeAllFacets(Ice.Util.stringToIdentity("id1"));
                    test(false);
                }
                catch (Ice.NotRegisteredException)
                {
                }
                fm = adapter.removeAllFacets(Ice.Util.stringToIdentity("id2"));
                test(fm.Count == 3);
                test(fm["f1"] == obj1);
                test(fm["f2"] == obj2);
                test(fm[""] == obj3);
                output.WriteLine("ok");

                adapter.deactivate();

                output.Write("testing stringToProxy... ");
                output.Flush();
                string @ref = "d:" + helper.getTestEndpoint(0);
                Ice.ObjectPrx db = communicator.stringToProxy(@ref);
                test(db != null);
                output.WriteLine("ok");

                output.Write("testing unchecked cast... ");
                output.Flush();
                Ice.ObjectPrx prx = Ice.ObjectPrxHelper.uncheckedCast(db);
                test(prx.ice_getFacet().Length == 0);
                prx = Ice.ObjectPrxHelper.uncheckedCast(db, "facetABCD");
                test(prx.ice_getFacet() == "facetABCD");
                Ice.ObjectPrx prx2 = Ice.ObjectPrxHelper.uncheckedCast(prx);
                test(prx2.ice_getFacet() == "facetABCD");
                Ice.ObjectPrx prx3 = Ice.ObjectPrxHelper.uncheckedCast(prx, "");
                test(prx3.ice_getFacet().Length == 0);
                var d = Test.DPrxHelper.uncheckedCast(db);
                test(d.ice_getFacet().Length == 0);
                var df = Test.DPrxHelper.uncheckedCast(db, "facetABCD");
                test(df.ice_getFacet() == "facetABCD");
                var df2 = Test.DPrxHelper.uncheckedCast(df);
                test(df2.ice_getFacet() == "facetABCD");
                var df3 = Test.DPrxHelper.uncheckedCast(df, "");
                test(df3.ice_getFacet().Length == 0);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
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
                output.WriteLine("ok");

                output.Write("testing non-facets A, B, C, and D... ");
                output.Flush();
                d = Test.DPrxHelper.checkedCast(db);
                test(d != null);
                test(d.Equals(db));
                test(d.callA().Equals("A"));
                test(d.callB().Equals("B"));
                test(d.callC().Equals("C"));
                test(d.callD().Equals("D"));
                output.WriteLine("ok");

                output.Write("testing facets A, B, C, and D... ");
                output.Flush();
                df = Test.DPrxHelper.checkedCast(d, "facetABCD");
                test(df != null);
                test(df.callA().Equals("A"));
                test(df.callB().Equals("B"));
                test(df.callC().Equals("C"));
                test(df.callD().Equals("D"));
                output.WriteLine("ok");

                output.Write("testing facets E and F... ");
                output.Flush();
                var ff = Test.FPrxHelper.checkedCast(d, "facetEF");
                test(ff != null);
                test(ff.callE().Equals("E"));
                test(ff.callF().Equals("F"));
                output.WriteLine("ok");

                output.Write("testing facet G... ");
                output.Flush();
                var gf = Test.GPrxHelper.checkedCast(ff, "facetGH");
                test(gf != null);
                test(gf.callG().Equals("G"));
                output.WriteLine("ok");

                output.Write("testing whether casting preserves the facet... ");
                output.Flush();
                var hf = Test.HPrxHelper.checkedCast(gf);
                test(hf != null);
                test(hf.callG().Equals("G"));
                test(hf.callH().Equals("H"));
                output.WriteLine("ok");
                return gf;
            }
        }
    }
}
