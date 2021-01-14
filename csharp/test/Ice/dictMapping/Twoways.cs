// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.DictMapping
{
    internal abstract class DictionaryComparer<TKey, TValue> : EqualityComparer<IReadOnlyDictionary<TKey, TValue>>
        where TKey : notnull
    {
        internal static DictionaryComparer<TKey, TValue> AllEntries { get; } = new AllEntriesDictionaryComparer();

        private class AllEntriesDictionaryComparer : DictionaryComparer<TKey, TValue>
        {
            public override bool Equals(
                IReadOnlyDictionary<TKey, TValue>? lhs,
                IReadOnlyDictionary<TKey, TValue>? rhs) =>
                ReferenceEquals(lhs, rhs) || (lhs != null && rhs != null && lhs.DictionaryEqual(rhs));

            public override int GetHashCode(IReadOnlyDictionary<TKey, TValue> obj) => obj.GetDictionaryHashCode();
        }
    }

    internal abstract class SequenceComparer<T> : EqualityComparer<IEnumerable<T>>
    {
        internal static SequenceComparer<T> AllElements { get; } = new AllElementsSequenceComparer();

        private class AllElementsSequenceComparer : SequenceComparer<T>
        {
            public override bool Equals(IEnumerable<T>? lhs, IEnumerable<T>? rhs) =>
                ReferenceEquals(lhs, rhs) || (lhs != null && rhs != null && lhs.SequenceEqual(rhs));

            public override int GetHashCode(IEnumerable<T> obj) => obj.GetSequenceHashCode();
        }
    }

    public static class Twoways
    {
        internal static Task RunAsync(IMyClassPrx p)
        {
            {
                var i = new Dictionary<int, int>
                {
                    [0] = 1,
                    [1] = 0
                };

                (Dictionary<int, int> r, Dictionary<int, int> o) = p.OpNV(i);

                TestHelper.Assert(i.DictionaryEqual(o));
                TestHelper.Assert(i.DictionaryEqual(r));
            }

            {
                var i = new Dictionary<string, string>
                {
                    ["a"] = "b",
                    ["b"] = "a"
                };

                (Dictionary<string, string> r, Dictionary<string, string> o) = p.OpNR(i);

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

                (Dictionary<string, Dictionary<int, int>> r, Dictionary<string, Dictionary<int, int>> o) = p.OpNDV(i);
                TestHelper.Assert(o.DictionaryEqual(i, DictionaryComparer<int, int>.AllEntries));
                TestHelper.Assert(r.DictionaryEqual(i, DictionaryComparer<int, int>.AllEntries));
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

                (Dictionary<string, Dictionary<string, string>> r,
                Dictionary<string, Dictionary<string, string>> o) = p.OpNDR(i);
                TestHelper.Assert(o.DictionaryEqual(i, DictionaryComparer<string, string>.AllEntries));
                TestHelper.Assert(r.DictionaryEqual(i, DictionaryComparer<string, string>.AllEntries));
            }

            {
                int[] ii = new int[] { 1, 2 };
                var i = new Dictionary<string, int[]>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                (Dictionary<string, int[]> r, Dictionary<string, int[]> o) = p.OpNDAIS(i);
                TestHelper.Assert(o.DictionaryEqual(i, SequenceComparer<int>.AllElements));
                TestHelper.Assert(r.DictionaryEqual(i, SequenceComparer<int>.AllElements));
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

                (Dictionary<string, List<int>> r, Dictionary<string, List<int>> o) = p.OpNDGIS(i);
                TestHelper.Assert(o.DictionaryEqual(i, SequenceComparer<int>.AllElements));
                TestHelper.Assert(r.DictionaryEqual(i, SequenceComparer<int>.AllElements));
            }

            {
                string[] ii = new string[] { "a", "b" };
                var i = new Dictionary<string, string[]>
                {
                    ["a"] = ii,
                    ["b"] = ii
                };

                (Dictionary<string, string[]> r, Dictionary<string, string[]> o) = p.OpNDASS(i);
                TestHelper.Assert(o.DictionaryEqual(i, SequenceComparer<string>.AllElements));
                TestHelper.Assert(r.DictionaryEqual(i, SequenceComparer<string>.AllElements));
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

                (Dictionary<string, List<string>> r, Dictionary<string, List<string>> o) = p.OpNDGSS(i);
                TestHelper.Assert(o.DictionaryEqual(i, SequenceComparer<string>.AllElements));
                TestHelper.Assert(r.DictionaryEqual(i, SequenceComparer<string>.AllElements));
            }
            return Task.CompletedTask;
        }
    }
}
