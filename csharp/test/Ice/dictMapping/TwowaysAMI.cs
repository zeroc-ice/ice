// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using Test;

namespace ZeroC.Ice.Test.DictMapping
{
    public static class TwowaysAMI
    {
        internal static void Run(IMyClassPrx p)
        {
            {
                var i = new Dictionary<int, int>
                {
                    [0] = 1,
                    [1] = 0
                };

                (Dictionary<int, int> ReturnValue, Dictionary<int, int> o) = p.OpNVAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i));
            }

            {
                var i = new Dictionary<string, string>
                {
                    ["a"] = "b",
                    ["b"] = "a"
                };

                (Dictionary<string, string> ReturnValue, Dictionary<string, string> o) = p.OpNRAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i));
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

                (Dictionary<string, Dictionary<int, int>> ReturnValue,
                 Dictionary<string, Dictionary<int, int>> o) = p.OpNDVAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i, (lhs, rhs) => lhs.DictionaryEquals(rhs)));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i, (lhs, rhs) => lhs.DictionaryEquals(rhs)));
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

                (Dictionary<string, Dictionary<string, string>> ReturnValue,
                 Dictionary<string, Dictionary<string, string>> o) = p.OpNDRAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i, (lhs, rhs) => lhs.DictionaryEquals(rhs)));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i, (lhs, rhs) => lhs.DictionaryEquals(rhs)));
            }

            {
                int[] ii = new int[] { 1, 2 };
                var i = new Dictionary<string, int[]>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                (Dictionary<string, int[]> ReturnValue, Dictionary<string, int[]> o) = p.OpNDAISAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
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

                (Dictionary<string, List<int>> ReturnValue,
                 Dictionary<string, List<int>> o) = p.OpNDGISAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
            }

            {
                string[] ii = new string[] { "a", "b" };
                var i = new Dictionary<string, string[]>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                (Dictionary<string, string[]> ReturnValue,
                 Dictionary<string, string[]> o) = p.OpNDASSAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
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

                (Dictionary<string, List<string>> ReturnValue,
                 Dictionary<string, List<string>> o) = p.OpNDGSSAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
            }
        }
    }
}
