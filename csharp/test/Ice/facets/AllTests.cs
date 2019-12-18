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
                test(communicator.GetPropertyAsList("Ice.Admin.Facets") == null);
                communicator.SetProperty("Ice.Admin.Facets", "foobar");
                string[]? facetFilter = communicator.GetPropertyAsList("Ice.Admin.Facets");
                test(facetFilter != null && facetFilter.Length == 1 && facetFilter[0].Equals("foobar"));
                communicator.SetProperty("Ice.Admin.Facets", "foo\\'bar");
                facetFilter = communicator.GetPropertyAsList("Ice.Admin.Facets");
                test(facetFilter != null && facetFilter.Length == 1 && facetFilter[0].Equals("foo'bar"));
                communicator.SetProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
                facetFilter = communicator.GetPropertyAsList("Ice.Admin.Facets");
                test(facetFilter != null && facetFilter.Length == 3 && facetFilter[0].Equals("foo bar") &&
                     facetFilter[1].Equals("toto") && facetFilter[2].Equals("titi"));
                communicator.SetProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
                facetFilter = communicator.GetPropertyAsList("Ice.Admin.Facets");
                test(facetFilter != null && facetFilter.Length == 2 && facetFilter[0].Equals("foo bar' toto") &&
                     facetFilter[1].Equals("titi"));
                // communicator.SetProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
                // facetFilter = communicator.Properties.getPropertyAsList("Ice.Admin.Facets");
                // test(facetFilter.Length == 0);
                communicator.SetProperty("Ice.Admin.Facets", "");
                output.WriteLine("ok");

                output.Write("testing facet registration exceptions... ");
                communicator.SetProperty("FacetExceptionTestAdapter.Endpoints", "tcp -h *");
                ObjectAdapter adapter = communicator.createObjectAdapter("FacetExceptionTestAdapter");

                var obj = new EmptyI();

                adapter.Add(obj, "d");
                adapter.Add(obj, "d", "facetABCD");
                try
                {
                    adapter.Add(obj, "d", "facetABCD");
                    test(false);
                }
                catch (AlreadyRegisteredException)
                {
                }
                adapter.Remove("d", "facetABCD");
                try
                {
                    adapter.Remove("d", "facetABCD");
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

                adapter.Add(obj1, "id1", "f1");
                adapter.Add(obj2, "id1", "f2");
                adapter.Add(obj1, "id2", "f1");
                adapter.Add(obj2, "id2", "f2");
                adapter.Add(obj3, "id2", "");
                Dictionary<string, Disp> fm = adapter.RemoveAllFacets("id1");
                test(fm.Count == 2);
                test(fm.ContainsKey("f1"));
                test(fm.ContainsKey("f2"));
                try
                {
                    adapter.RemoveAllFacets("id1");
                    test(false);
                }
                catch (NotRegisteredException)
                {
                }
                fm = adapter.RemoveAllFacets("id2");
                test(fm.Count == 3);
                test(fm.ContainsKey("f1"));
                test(fm.ContainsKey("f2"));
                test(fm.ContainsKey(""));
                output.WriteLine("ok");

                adapter.Deactivate();

                var prx = IObjectPrx.Parse($"d:{helper.getTestEndpoint(0)}", communicator);

                output.Write("testing unchecked cast... ");
                output.Flush();
                var d = DPrx.UncheckedCast(prx);
                test(d.Facet.Length == 0);
                var df = DPrx.UncheckedCast(prx.Clone(facet: "facetABCD"));
                test(df.Facet == "facetABCD");
                var df2 = DPrx.UncheckedCast(df);
                test(df2.Facet == "facetABCD");
                var df3 = DPrx.UncheckedCast(df.Clone(facet: ""));
                test(df3.Facet.Length == 0);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                d = DPrx.CheckedCast(prx);
                test(d.Facet.Length == 0);
                df = DPrx.CheckedCast(prx.Clone(facet: "facetABCD"));
                test(df.Facet == "facetABCD");
                df2 = DPrx.CheckedCast(df);
                test(df2.Facet == "facetABCD");
                df3 = DPrx.CheckedCast(df.Clone(facet: ""));
                test(df3.Facet.Length == 0);
                output.WriteLine("ok");

                output.Write("testing non-facets A, B, C, and D... ");
                output.Flush();
                d = DPrx.CheckedCast(prx);
                test(d != null);
                test(d.Equals(prx));
                test(d.callA().Equals("A"));
                test(d.callB().Equals("B"));
                test(d.callC().Equals("C"));
                test(d.callD().Equals("D"));
                output.WriteLine("ok");

                output.Write("testing facets A, B, C, and D... ");
                output.Flush();
                df = DPrx.CheckedCast(d.Clone(facet: "facetABCD"));
                test(df != null);
                test(df.callA().Equals("A"));
                test(df.callB().Equals("B"));
                test(df.callC().Equals("C"));
                test(df.callD().Equals("D"));
                output.WriteLine("ok");

                output.Write("testing facets E and F... ");
                output.Flush();
                var ff = FPrx.CheckedCast(d.Clone(facet: "facetEF"));
                test(ff.callE().Equals("E"));
                test(ff.callF().Equals("F"));
                output.WriteLine("ok");

                output.Write("testing facet G... ");
                output.Flush();
                var gf = GPrx.CheckedCast(ff.Clone(facet: "facetGH"));
                test(gf.callG().Equals("G"));
                output.WriteLine("ok");

                output.Write("testing whether casting preserves the facet... ");
                output.Flush();
                var hf = HPrx.CheckedCast(gf);
                test(hf.callG().Equals("G"));
                test(hf.callH().Equals("H"));
                output.WriteLine("ok");
                return gf;
            }
        }
    }
}
