//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Ice.facets.Test;

namespace Ice
{
    namespace facets
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.GPrx allTests(global::Test.TestHelper helper)
            {

                Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing Ice.Admin.Facets property... ");
                test(communicator.getProperties().getPropertyAsList("Ice.Admin.Facets").Length == 0);
                communicator.getProperties().setProperty("Ice.Admin.Facets", "foobar");
                string[] facetFilter = communicator.getProperties().getPropertyAsList("Ice.Admin.Facets");
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
                ObjectAdapter adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter");

                var emptyT = default(EmptyTraits);
                var obj = new EmptyI();

                adapter.Add(obj, Util.stringToIdentity("d"));
                adapter.Add(obj, Util.stringToIdentity("d"), "facetABCD");
                try
                {
                    adapter.Add(obj, Util.stringToIdentity("d"), "facetABCD");
                    test(false);
                }
                catch (AlreadyRegisteredException)
                {
                }
                adapter.remove(Util.stringToIdentity("d"), "facetABCD");
                try
                {
                    adapter.remove(Util.stringToIdentity("d"), "facetABCD");
                    test(false);
                }
                catch (NotRegisteredException)
                {
                }
                output.WriteLine("ok");

                output.Write("testing removeAllFacets... ");
                var obj1 = new EmptyI();
                var obj2 = new EmptyI();
                var obj3 = new EmptyI();

                adapter.Add(obj1, Util.stringToIdentity("id1"), "f1");
                adapter.Add(obj2, Util.stringToIdentity("id1"), "f2");
                adapter.Add(obj1, Util.stringToIdentity("id2"), "f1");
                adapter.Add(obj2, Util.stringToIdentity("id2"), "f2");
                adapter.Add(obj3, Util.stringToIdentity("id2"), "");
                Dictionary<string, Disp> fm = adapter.removeAllFacets(Ice.Util.stringToIdentity("id1"));
                test(fm.Count == 2);
                test(fm.ContainsKey("f1"));
                test(fm.ContainsKey("f2"));
                try
                {
                    adapter.removeAllFacets(Util.stringToIdentity("id1"));
                    test(false);
                }
                catch (NotRegisteredException)
                {
                }
                fm = adapter.removeAllFacets(Util.stringToIdentity("id2"));
                test(fm.Count == 3);
                test(fm.ContainsKey("f1"));
                test(fm.ContainsKey("f2"));
                test(fm.ContainsKey(""));
                output.WriteLine("ok");

                adapter.deactivate();

                output.Write("testing stringToProxy... ");
                output.Flush();
                string @ref = "d:" + helper.getTestEndpoint(0);
                ObjectPrx db = communicator.stringToProxy(@ref);
                test(db != null);
                output.WriteLine("ok");

                output.Write("testing unchecked cast... ");
                output.Flush();
                ObjectPrx prx = Ice.ObjectPrxHelper.uncheckedCast(db);
                test(prx.ice_getFacet().Length == 0);
                prx = Ice.ObjectPrxHelper.uncheckedCast(db, "facetABCD");
                test(prx.ice_getFacet() == "facetABCD");
                ObjectPrx prx2 = Ice.ObjectPrxHelper.uncheckedCast(prx);
                test(prx2.ice_getFacet() == "facetABCD");
                ObjectPrx prx3 = Ice.ObjectPrxHelper.uncheckedCast(prx, "");
                test(prx3.ice_getFacet().Length == 0);
                var d = Test.DPrxHelper.uncheckedCast(db);
                test(d.ice_getFacet().Length == 0);
                var df = DPrxHelper.uncheckedCast(db, "facetABCD");
                test(df.ice_getFacet() == "facetABCD");
                var df2 = DPrxHelper.uncheckedCast(df);
                test(df2.ice_getFacet() == "facetABCD");
                var df3 = DPrxHelper.uncheckedCast(df, "");
                test(df3.ice_getFacet().Length == 0);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                prx = ObjectPrxHelper.checkedCast(db);
                test(prx.ice_getFacet().Length == 0);
                prx = ObjectPrxHelper.checkedCast(db, "facetABCD");
                test(prx.ice_getFacet() == "facetABCD");
                prx2 = ObjectPrxHelper.checkedCast(prx);
                test(prx2.ice_getFacet() == "facetABCD");
                prx3 = ObjectPrxHelper.checkedCast(prx, "");
                test(prx3.ice_getFacet().Length == 0);
                d = DPrxHelper.checkedCast(db);
                test(d.ice_getFacet().Length == 0);
                df = DPrxHelper.checkedCast(db, "facetABCD");
                test(df.ice_getFacet() == "facetABCD");
                df2 = Test.DPrxHelper.checkedCast(df);
                test(df2.ice_getFacet() == "facetABCD");
                df3 = DPrxHelper.checkedCast(df, "");
                test(df3.ice_getFacet().Length == 0);
                output.WriteLine("ok");

                output.Write("testing non-facets A, B, C, and D... ");
                output.Flush();
                d = DPrxHelper.checkedCast(db);
                test(d != null);
                test(d.Equals(db));
                test(d.callA().Equals("A"));
                test(d.callB().Equals("B"));
                test(d.callC().Equals("C"));
                test(d.callD().Equals("D"));
                output.WriteLine("ok");

                output.Write("testing facets A, B, C, and D... ");
                output.Flush();
                df = DPrxHelper.checkedCast(d, "facetABCD");
                test(df != null);
                test(df.callA().Equals("A"));
                test(df.callB().Equals("B"));
                test(df.callC().Equals("C"));
                test(df.callD().Equals("D"));
                output.WriteLine("ok");

                output.Write("testing facets E and F... ");
                output.Flush();
                var ff = FPrxHelper.checkedCast(d, "facetEF");
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
