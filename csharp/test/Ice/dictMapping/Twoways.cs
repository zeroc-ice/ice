//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

using Test;

namespace Ice.dictMapping
{
    public class Twoways
    {
        internal static void twoways(Test.IMyClassPrx p)
        {
            {
                var i = new Dictionary<int, int>
                {
                    [0] = 1,
                    [1] = 0
                };

                Dictionary<int, int> o;
                Dictionary<int, int> r;
                (r, o) = p.opNV(i);

                TestHelper.Assert(i.DictionaryEqual(o));
                TestHelper.Assert(i.DictionaryEqual(r));
            }

            {
                var i = new Dictionary<string, string>
                {
                    ["a"] = "b",
                    ["b"] = "a"
                };

                Dictionary<string, string> o;
                Dictionary<string, string> r;
                (r, o) = p.opNR(i);

                TestHelper.Assert(i.DictionaryEqual(o));
                TestHelper.Assert(i.DictionaryEqual(r));
            }

            {
                var i = new Dictionary<string, Dictionary<int, int>>();
                var id = new Dictionary<int, int>
                {
                    [0] = 1,
                    [1] = 0
                };
                i["a"] = id;
                i["b"] = id;

                Dictionary<string, Dictionary<int, int>> o;
                Dictionary<string, Dictionary<int, int>> r;
                (r, o) = p.opNDV(i);

                foreach (string key in i.Keys)
                {
                    TestHelper.Assert(i[key].DictionaryEqual(o[key]));
                    TestHelper.Assert(i[key].DictionaryEqual(r[key]));
                }
            }

            {
                var i = new Dictionary<string, Dictionary<string, string>>();
                var id = new Dictionary<string, string>
                {
                    ["a"] = "b",
                    ["b"] = "a"
                };
                i["a"] = id;
                i["b"] = id;

                Dictionary<string, Dictionary<string, string>> o;
                Dictionary<string, Dictionary<string, string>> r;
                (r, o) = p.opNDR(i);

                foreach (string key in i.Keys)
                {
                    TestHelper.Assert(i[key].DictionaryEqual(o[key]));
                    TestHelper.Assert(i[key].DictionaryEqual(r[key]));
                }
            }

            {
                int[] ii = new int[] { 1, 2 };
                var i = new Dictionary<string, int[]>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                Dictionary<string, int[]> o;
                Dictionary<string, int[]> r;
                (r, o) = p.opNDAIS(i);

                foreach (string key in i.Keys)
                {
                    TestHelper.Assert(i[key].SequenceEqual(o[key]));
                    TestHelper.Assert(i[key].SequenceEqual(r[key]));
                }
            }

            {
                var ii = new List<int>
                {
                    1,
                    2
                };
                var i = new Dictionary<string, List<int>>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                Dictionary<string, List<int>> o;
                Dictionary<string, List<int>> r;
                (r, o) = p.opNDGIS(i);

                foreach (string key in i.Keys)
                {
                    TestHelper.Assert(i[key].SequenceEqual(o[key]));
                    TestHelper.Assert(i[key].SequenceEqual(r[key]));
                }
            }

            {
                string[] ii = new string[] { "a", "b" };
                var i = new Dictionary<string, string[]>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                Dictionary<string, string[]> o;
                Dictionary<string, string[]> r;
                (r, o) = p.opNDASS(i);

                foreach (string key in i.Keys)
                {
                    TestHelper.Assert(i[key].SequenceEqual(o[key]));
                    TestHelper.Assert(i[key].SequenceEqual(r[key]));
                }
            }

            {
                var ii = new List<string>
                {
                    "a",
                    "b"
                };
                var i = new Dictionary<string, List<string>>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                Dictionary<string, List<string>> o;
                Dictionary<string, List<string>> r;
                (r, o) = p.opNDGSS(i);

                foreach (string key in i.Keys)
                {
                    TestHelper.Assert(i[key].SequenceEqual(o[key]));
                    TestHelper.Assert(i[key].SequenceEqual(r[key]));
                }
            }
        }
    }
}
