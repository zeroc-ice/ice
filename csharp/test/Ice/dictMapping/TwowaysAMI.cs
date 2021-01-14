// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.DictMapping
{
    public static class TwowaysAMI
    {
        internal static Task RunAsync(IMyClassPrx p)
        {
            {
                var i = new Dictionary<int, int>
                {
                    [0] = 1,
                    [1] = 0
                };

                (Dictionary<int, int> ReturnValue, Dictionary<int, int> o) = p.OpNVAsync(i).Result;
                TestHelper.Assert(o.DictionaryEqual(i));
                TestHelper.Assert(ReturnValue.DictionaryEqual(i));
            }

            {
                var i = new Dictionary<string, string>
                {
                    ["a"] = "b",
                    ["b"] = "a"
                };

                (Dictionary<string, string> ReturnValue, Dictionary<string, string> o) = p.OpNRAsync(i).Result;
                TestHelper.Assert(o.DictionaryEqual(i));
                TestHelper.Assert(ReturnValue.DictionaryEqual(i));
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
                TestHelper.Assert(o.DictionaryEqual(i, DictionaryComparer<int, int>.AllEntries));
                TestHelper.Assert(ReturnValue.DictionaryEqual(i, DictionaryComparer<int, int>.AllEntries));
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
                TestHelper.Assert(o.DictionaryEqual(i, DictionaryComparer<string, string>.AllEntries));
                TestHelper.Assert(ReturnValue.DictionaryEqual(i, DictionaryComparer<string, string>.AllEntries));
            }

            {
                int[] ii = new int[] { 1, 2 };
                var i = new Dictionary<string, int[]>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                (Dictionary<string, int[]> ReturnValue, Dictionary<string, int[]> o) = p.OpNDAISAsync(i).Result;
                TestHelper.Assert(o.DictionaryEqual(i, SequenceComparer<int>.AllElements));
                TestHelper.Assert(ReturnValue.DictionaryEqual(i, SequenceComparer<int>.AllElements));
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
                TestHelper.Assert(o.DictionaryEqual(i, SequenceComparer<int>.AllElements));
                TestHelper.Assert(ReturnValue.DictionaryEqual(i, SequenceComparer<int>.AllElements));
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
                TestHelper.Assert(o.DictionaryEqual(i, SequenceComparer<string>.AllElements));
                TestHelper.Assert(ReturnValue.DictionaryEqual(i, SequenceComparer<string>.AllElements));
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
                TestHelper.Assert(o.DictionaryEqual(i, SequenceComparer<string>.AllElements));
                TestHelper.Assert(ReturnValue.DictionaryEqual(i, SequenceComparer<string>.AllElements));
            }
            return Task.CompletedTask;
        }
    }
}
