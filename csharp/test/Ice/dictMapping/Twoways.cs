// Copyright (c) ZeroC, Inc.

namespace Ice.dictMapping;

internal class Twoways
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    internal static void twoways(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        {
            var i = new Dictionary<int, int>
            {
                [0] = 1,
                [1] = 0
            };

            Dictionary<int, int> r;
            r = p.opNV(i, out Dictionary<int, int> o);

            test(Internal.DictionaryExtensions.DictionaryEqual(i, o));
            test(Internal.DictionaryExtensions.DictionaryEqual(i, r));
        }

        {
            var i = new Dictionary<string, string>
            {
                ["a"] = "b",
                ["b"] = "a"
            };

            Dictionary<string, string> r;
            r = p.opNR(i, out Dictionary<string, string> o);

            test(Internal.DictionaryExtensions.DictionaryEqual(i, o));
            test(Internal.DictionaryExtensions.DictionaryEqual(i, r));
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

            Dictionary<string, Dictionary<int, int>> r;
            r = p.opNDV(i, out Dictionary<string, Dictionary<int, int>> o);

            foreach (string key in i.Keys)
            {
                test(Internal.DictionaryExtensions.DictionaryEqual(i[key], o[key]));
                test(Internal.DictionaryExtensions.DictionaryEqual(i[key], r[key]));
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

            Dictionary<string, Dictionary<string, string>> r;
            r = p.opNDR(i, out Dictionary<string, Dictionary<string, string>> o);

            foreach (string key in i.Keys)
            {
                test(Internal.DictionaryExtensions.DictionaryEqual(i[key], o[key]));
                test(Internal.DictionaryExtensions.DictionaryEqual(i[key], r[key]));
            }
        }

        {
            int[] ii = new int[] { 1, 2 };
            var i = new Dictionary<string, int[]>
            {
                ["a"] = ii,
                ["b"] = ii
            };

            Dictionary<string, int[]> r;
            r = p.opNDAIS(i, out Dictionary<string, int[]> o);

            foreach (string key in i.Keys)
            {
                test(Enumerable.SequenceEqual(i[key], o[key]));
                test(Enumerable.SequenceEqual(i[key], r[key]));
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

            Dictionary<string, List<int>> r;
            r = p.opNDGIS(i, out Dictionary<string, List<int>> o);

            foreach (string key in i.Keys)
            {
                test(Enumerable.SequenceEqual(i[key], o[key]));
                test(Enumerable.SequenceEqual(i[key], r[key]));
            }
        }

        {
            string[] ii = new string[] { "a", "b" };
            var i = new Dictionary<string, string[]>
            {
                ["a"] = ii,
                ["b"] = ii
            };

            Dictionary<string, string[]> r;
            r = p.opNDASS(i, out Dictionary<string, string[]> o);

            foreach (string key in i.Keys)
            {
                test(Enumerable.SequenceEqual(i[key], o[key]));
                test(Enumerable.SequenceEqual(i[key], r[key]));
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

            Dictionary<string, List<string>> r;
            r = p.opNDGSS(i, out Dictionary<string, List<string>> o);

            foreach (string key in i.Keys)
            {
                test(Enumerable.SequenceEqual(i[key], o[key]));
                test(Enumerable.SequenceEqual(i[key], r[key]));
            }
        }
    }
}
