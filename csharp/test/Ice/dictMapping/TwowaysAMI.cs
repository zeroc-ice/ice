//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Collections.Generic;
using System.Linq;

namespace Ice.dictMapping
{
    public static class DictionaryExtension
    {
        public static bool DictionaryEquals<TKey, TValue>(this Dictionary<TKey, TValue> self,
                                                          Dictionary<TKey, TValue> other) where TKey : notnull
                                                                                          where TValue : notnull
        {
            if (self.Count != other.Count)
            {
                return false;
            }

            foreach (KeyValuePair<TKey, TValue> entry in self)
            {

                if (!other.TryGetValue(entry.Key, out TValue value))
                {
                    return false;
                }

                if (!value.Equals(entry.Value))
                {
                    return false;
                }
            }
            return true;
        }

        public static bool DictionaryEquals<TKey, TValue>(
            this Dictionary<TKey, TValue> self,
            Dictionary<TKey, TValue> other,
            System.Func<TValue, TValue, bool> equals) where TKey : notnull
        {
            if (self.Count != other.Count)
            {
                return false;
            }

            foreach (KeyValuePair<TKey, TValue> entry in self)
            {

                if (!other.TryGetValue(entry.Key, out TValue value))
                {
                    return false;
                }

                if (!equals(value, entry.Value))
                {
                    return false;
                }
            }
            return true;
        }
    }
    public class TwowaysAMI
    {
        private class CallbackBase
        {
            internal CallbackBase() => _called = false;

            public virtual void Check()
            {
                lock (this)
                {
                    while (!_called)
                    {
                        System.Threading.Monitor.Wait(this);
                    }

                    _called = false;
                }
            }

            public virtual void Called()
            {
                lock (this)
                {
                    TestHelper.Assert(!_called);
                    _called = true;
                    System.Threading.Monitor.Pulse(this);
                }
            }

            private bool _called;
        }

        internal static void twowaysAMI(Test.IMyClassPrx p)
        {
            {
                var i = new Dictionary<int, int>
                {
                    [0] = 1,
                    [1] = 0
                };

                (Dictionary<int, int> ReturnValue, Dictionary<int, int> o) = p.opNVAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i));
            }

            {
                var i = new Dictionary<string, string>
                {
                    ["a"] = "b",
                    ["b"] = "a"
                };

                (Dictionary<string, string> ReturnValue, Dictionary<string, string> o) = p.opNRAsync(i).Result;
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
                 Dictionary<string, Dictionary<int, int>> o) = p.opNDVAsync(i).Result;
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
                 Dictionary<string, Dictionary<string, string>> o) = p.opNDRAsync(i).Result;
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

                (Dictionary<string, int[]> ReturnValue, Dictionary<string, int[]> o) = p.opNDAISAsync(i).Result;
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
                 Dictionary<string, List<int>> o) = p.opNDGISAsync(i).Result;
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
                 Dictionary<string, string[]> o) = p.opNDASSAsync(i).Result;
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
                 Dictionary<string, List<string>> o) = p.opNDGSSAsync(i).Result;
                TestHelper.Assert(o.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
                TestHelper.Assert(ReturnValue.DictionaryEquals(i, (lhs, rhs) => lhs.SequenceEqual(rhs)));
            }
        }
    }
}
