// Copyright (c) ZeroC, Inc.

namespace Ice
{
    namespace dictMapping
    {
        public class TwowaysAMI
        {
            private static void test(bool b) => global::Test.TestHelper.test(b);

            internal static async Task twowaysAMI(Test.MyClassPrx p)
            {
                {
                    var i = new Dictionary<int, int>
                    {
                        [0] = 1,
                        [1] = 0
                    };

                    var result = await p.opNVAsync(i);
                    test(Internal.DictionaryExtensions.DictionaryEqual(i, result.o));
                    test(Internal.DictionaryExtensions.DictionaryEqual(i, result.returnValue));
                }

                {
                    var i = new Dictionary<string, string>
                    {
                        ["a"] = "b",
                        ["b"] = "a"
                    };

                    var result = await p.opNRAsync(i);
                    test(Internal.DictionaryExtensions.DictionaryEqual(i, result.o));
                    test(Internal.DictionaryExtensions.DictionaryEqual(i, result.returnValue));
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

                    var result = await p.opNDVAsync(i);
                    foreach (string key in i.Keys)
                    {
                        test(Internal.DictionaryExtensions.DictionaryEqual(i[key], result.o[key]));
                        test(Internal.DictionaryExtensions.DictionaryEqual(i[key], result.returnValue[key]));
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

                    var result = await p.opNDRAsync(i);
                    foreach (string key in i.Keys)
                    {
                        test(Internal.DictionaryExtensions.DictionaryEqual(i[key], result.o[key]));
                        test(Internal.DictionaryExtensions.DictionaryEqual(i[key], result.returnValue[key]));
                    }
                }

                {
                    int[] ii = new int[] { 1, 2 };
                    var i = new Dictionary<string, int[]>
                    {
                        ["a"] = ii,
                        ["b"] = ii
                    };

                    var result = await p.opNDAISAsync(i);
                    foreach (string key in i.Keys)
                    {
                        test(Enumerable.SequenceEqual(i[key], result.o[key]));
                        test(Enumerable.SequenceEqual(i[key], result.returnValue[key]));
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

                    var result = await p.opNDGISAsync(i);
                    foreach (string key in i.Keys)
                    {
                        test(Enumerable.SequenceEqual(i[key], result.o[key]));
                        test(Enumerable.SequenceEqual(i[key], result.returnValue[key]));
                    }
                }

                {
                    string[] ii = new string[] { "a", "b" };
                    var i = new Dictionary<string, string[]>
                    {
                        ["a"] = ii,
                        ["b"] = ii
                    };

                    var result = await p.opNDASSAsync(i);
                    foreach (string key in i.Keys)
                    {
                        test(Enumerable.SequenceEqual(i[key], result.o[key]));
                        test(Enumerable.SequenceEqual(i[key], result.returnValue[key]));
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

                    var result = await p.opNDGSSAsync(i);
                    foreach (string key in i.Keys)
                    {
                        test(Enumerable.SequenceEqual(i[key], result.o[key]));
                        test(Enumerable.SequenceEqual(i[key], result.returnValue[key]));
                    }
                }
            }
        }
    }
}
