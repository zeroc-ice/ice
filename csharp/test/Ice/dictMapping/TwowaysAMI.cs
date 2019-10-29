//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;

namespace Ice
{
    namespace dictMapping
    {
        public static class DictionaryExtension
        {
            public static bool DictionaryEquals<K, V>(this Dictionary<K, V> self, Dictionary<K, V> other)
            {
                if (self.Count != other.Count)
                {
                    return false;
                }

                foreach (var entry in self)
                {
                    V value;

                    if (!other.TryGetValue(entry.Key, out value))
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

            public static bool DictionaryEquals<K, V>(this Dictionary<K, V> self, Dictionary<K, V> other, System.Func<V, V, bool> equals)
            {
                if (self.Count != other.Count)
                {
                    return false;
                }

                foreach (var entry in self)
                {
                    V value;

                    if (!other.TryGetValue(entry.Key, out value))
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
            private static void test(bool b)
            {
                if (!b)
                {
                    throw new System.SystemException();
                }
            }

            private class CallbackBase
            {
                internal CallbackBase()
                {
                    _called = false;
                }

                public virtual void check()
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

                public virtual void called()
                {
                    lock (this)
                    {
                        Debug.Assert(!_called);
                        _called = true;
                        System.Threading.Monitor.Pulse(this);
                    }
                }

                private bool _called;
            }

            internal static void twowaysAMI(Communicator communicator, Test.MyClassPrx p)
            {
                {
                    Dictionary<int, int> i = new Dictionary<int, int>();
                    i[0] = 1;
                    i[1] = 0;

                    var r = p.opNVAsync(i).Result;
                    test(r.o.DictionaryEquals(i));
                    test(r.returnValue.DictionaryEquals(i));
                }

                {
                    Dictionary<string, string> i = new Dictionary<string, string>();
                    i["a"] = "b";
                    i["b"] = "a";

                    var r = p.opNRAsync(i).Result;
                    test(r.o.DictionaryEquals(i));
                    test(r.returnValue.DictionaryEquals(i));
                }

                {
                    Dictionary<string, Dictionary<int, int>> i = new Dictionary<string, Dictionary<int, int>>();
                    Dictionary<int, int> id = new Dictionary<int, int>();
                    id[0] = 1;
                    id[1] = 0;
                    i["a"] = id;
                    i["b"] = id;

                    var r = p.opNDVAsync(i).Result;
                    test(r.o.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.DictionaryEquals(rhs);
                        }));
                    test(r.returnValue.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.DictionaryEquals(rhs);
                        }));
                }

                {
                    Dictionary<string, Dictionary<string, string>> i = new Dictionary<string, Dictionary<string, string>>();
                    Dictionary<string, string> id = new Dictionary<string, string>();
                    id["a"] = "b";
                    id["b"] = "a";
                    i["a"] = id;
                    i["b"] = id;

                    var r = p.opNDRAsync(i).Result;
                    test(r.o.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.DictionaryEquals(rhs);
                        }));
                    test(r.returnValue.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.DictionaryEquals(rhs);
                        }));
                }

                {
                    int[] ii = new int[] { 1, 2 };
                    Dictionary<string, int[]> i = new Dictionary<string, int[]>();
                    i["a"] = ii;
                    i["b"] = ii;

                    var r = p.opNDAISAsync(i).Result;
                    test(r.o.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.SequenceEqual(rhs);
                        }));
                    test(r.returnValue.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.SequenceEqual(rhs);
                        }));
                }

                {
                    List<int> ii = new List<int>();
                    ii.Add(1);
                    ii.Add(2);
                    Dictionary<string, List<int>> i = new Dictionary<string, List<int>>();
                    i["a"] = ii;
                    i["b"] = ii;

                    var r = p.opNDGISAsync(i).Result;
                    test(r.o.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.SequenceEqual(rhs);
                        }));
                    test(r.returnValue.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.SequenceEqual(rhs);
                        }));
                }

                {
                    string[] ii = new string[] { "a", "b" };
                    Dictionary<string, string[]> i = new Dictionary<string, string[]>();
                    i["a"] = ii;
                    i["b"] = ii;

                    var r = p.opNDASSAsync(i).Result;
                    test(r.o.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.SequenceEqual(rhs);
                        }));
                    test(r.returnValue.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.SequenceEqual(rhs);
                        }));
                }

                {
                    List<string> ii = new List<string>();
                    ii.Add("a");
                    ii.Add("b");
                    Dictionary<string, List<string>> i = new Dictionary<string, List<string>>();
                    i["a"] = ii;
                    i["b"] = ii;

                    var r = p.opNDGSSAsync(i).Result;
                    test(r.o.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.SequenceEqual(rhs);
                        }));
                    test(r.returnValue.DictionaryEquals(i, (lhs, rhs) =>
                        {
                            return lhs.SequenceEqual(rhs);
                        }));
                }
            }
        }
    }
}
