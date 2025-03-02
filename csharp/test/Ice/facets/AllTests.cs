// Copyright (c) ZeroC, Inc.

namespace Ice
{
    namespace facets
    {
        public class AllTests : global::Test.AllTests
        {
            public static async Task<Test.GPrx> allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing Ice.Admin.Facets property... ");
                test(communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets").Length == 0);
                communicator.getProperties().setProperty("Ice.Admin.Facets", "foobar");
                String[] facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
                test(facetFilter.Length == 1 && facetFilter[0] == "foobar");
                communicator.getProperties().setProperty("Ice.Admin.Facets", "foo\\'bar");
                facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
                test(facetFilter.Length == 1 && facetFilter[0] == "foo'bar");
                communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
                facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
                test(facetFilter.Length == 3 && facetFilter[0] == "foo bar" && facetFilter[1] == "toto"
                     && facetFilter[2] == "titi");
                communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
                facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
                test(facetFilter.Length == 2 && facetFilter[0] == "foo bar' toto" && facetFilter[1] == "titi");
                // communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
                // facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
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
                prx = await Ice.ObjectPrxHelper.checkedCastAsync(db);
                test(prx.ice_getFacet().Length == 0);
                prx = await Ice.ObjectPrxHelper.checkedCastAsync(db, "facetABCD");
                test(prx.ice_getFacet() == "facetABCD");
                prx2 = await Ice.ObjectPrxHelper.checkedCastAsync(prx);
                test(prx2.ice_getFacet() == "facetABCD");
                prx3 = await Ice.ObjectPrxHelper.checkedCastAsync(prx, "");
                test(prx3.ice_getFacet().Length == 0);
                d = await Test.DPrxHelper.checkedCastAsync(db);
                test(d.ice_getFacet().Length == 0);
                df = await Test.DPrxHelper.checkedCastAsync(db, "facetABCD");
                test(df.ice_getFacet() == "facetABCD");
                df2 = await Test.DPrxHelper.checkedCastAsync(df);
                test(df2.ice_getFacet() == "facetABCD");
                df3 = await Test.DPrxHelper.checkedCastAsync(df, "");
                test(df3.ice_getFacet().Length == 0);
                output.WriteLine("ok");

                output.Write("testing non-facets A, B, C, and D... ");
                output.Flush();
                d = await Test.DPrxHelper.checkedCastAsync(db);
                test(d != null);
                test(d.Equals(db));
                test(d.callA() == "A");
                test(d.callB() == "B");
                test(d.callC() == "C");
                test(d.callD() == "D");
                output.WriteLine("ok");

                output.Write("testing facets A, B, C, and D... ");
                output.Flush();
                df = await Test.DPrxHelper.checkedCastAsync(d, "facetABCD");
                test(df != null);
                test(df.callA() == "A");
                test(df.callB() == "B");
                test(df.callC() == "C");
                test(df.callD() == "D");
                output.WriteLine("ok");

                output.Write("testing facets E and F... ");
                output.Flush();
                var ff = await Test.FPrxHelper.checkedCastAsync(d, "facetEF");
                test(ff != null);
                test(ff.callE() == "E");
                test(ff.callF() == "F");
                output.WriteLine("ok");

                output.Write("testing facet G... ");
                output.Flush();
                var gf = await Test.GPrxHelper.checkedCastAsync(ff, "facetGH");
                test(gf != null);
                test(gf.callG() == "G");
                output.WriteLine("ok");

                output.Write("testing whether casting preserves the facet... ");
                output.Flush();
                var hf = await Test.HPrxHelper.checkedCastAsync(gf);
                test(hf != null);
                test(hf.callG() == "G");
                test(hf.callH() == "H");
                output.WriteLine("ok");
                return gf;
            }
        }
    }
}
