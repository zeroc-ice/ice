//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace dictMapping
    {
        class Twoways
        {
            private static void test(bool b)
            {
                if (!b)
                {
                    throw new System.Exception();
                }
            }

            internal static void twoways(Ice.Communicator communicator, Test.IMyClassPrx p)
            {
                {
                    Dictionary<int, int> i = new Dictionary<int, int>();
                    i[0] = 1;
                    i[1] = 0;

                    Dictionary<int, int> o;
                    Dictionary<int, int> r;
                    (r, o) = p.opNV(i);

                    test(Ice.Collections.Equals(i, o));
                    test(Ice.Collections.Equals(i, r));
                }

                {
                    Dictionary<string, string> i = new Dictionary<string, string>();
                    i["a"] = "b";
                    i["b"] = "a";

                    Dictionary<string, string> o;
                    Dictionary<string, string> r;
                    (r, o) = p.opNR(i);

                    test(Ice.Collections.Equals(i, o));
                    test(Ice.Collections.Equals(i, r));
                }

                {
                    Dictionary<string, Dictionary<int, int>> i = new Dictionary<string, Dictionary<int, int>>();
                    Dictionary<int, int> id = new Dictionary<int, int>();
                    id[0] = 1;
                    id[1] = 0;
                    i["a"] = id;
                    i["b"] = id;

                    Dictionary<string, Dictionary<int, int>> o;
                    Dictionary<string, Dictionary<int, int>> r;
                    (r, o) = p.opNDV(i);

                    foreach (string key in i.Keys)
                    {
                        test(Ice.Collections.Equals(i[key], o[key]));
                        test(Ice.Collections.Equals(i[key], r[key]));
                    }
                }

                {
                    Dictionary<string, Dictionary<string, string>> i = new Dictionary<string, Dictionary<string, string>>();
                    Dictionary<string, string> id = new Dictionary<string, string>();
                    id["a"] = "b";
                    id["b"] = "a";
                    i["a"] = id;
                    i["b"] = id;

                    Dictionary<string, Dictionary<string, string>> o;
                    Dictionary<string, Dictionary<string, string>> r;
                    (r, o) = p.opNDR(i);

                    foreach (string key in i.Keys)
                    {
                        test(Ice.Collections.Equals(i[key], o[key]));
                        test(Ice.Collections.Equals(i[key], r[key]));
                    }
                }

                {
                    int[] ii = new int[] { 1, 2 };
                    Dictionary<string, int[]> i = new Dictionary<string, int[]>();
                    i["a"] = ii;
                    i["b"] = ii;

                    Dictionary<string, int[]> o;
                    Dictionary<string, int[]> r;
                    (r, o) = p.opNDAIS(i);

                    foreach (string key in i.Keys)
                    {
                        test(Ice.Collections.Equals(i[key], o[key]));
                        test(Ice.Collections.Equals(i[key], r[key]));
                    }
                }

                {
                    List<int> ii = new List<int>();
                    ii.Add(1);
                    ii.Add(2);
                    Dictionary<string, List<int>> i = new Dictionary<string, List<int>>();
                    i["a"] = ii;
                    i["b"] = ii;

                    Dictionary<string, List<int>> o;
                    Dictionary<string, List<int>> r;
                    (r, o) = p.opNDGIS(i);

                    foreach (string key in i.Keys)
                    {
                        test(Ice.Collections.Equals(i[key], o[key]));
                        test(Ice.Collections.Equals(i[key], r[key]));
                    }
                }

                {
                    string[] ii = new string[] { "a", "b" };
                    Dictionary<string, string[]> i = new Dictionary<string, string[]>();
                    i["a"] = ii;
                    i["b"] = ii;

                    Dictionary<string, string[]> o;
                    Dictionary<string, string[]> r;
                    (r, o) = p.opNDASS(i);

                    foreach (string key in i.Keys)
                    {
                        test(Ice.Collections.Equals(i[key], o[key]));
                        test(Ice.Collections.Equals(i[key], r[key]));
                    }
                }

                {
                    List<string> ii = new List<string>();
                    ii.Add("a");
                    ii.Add("b");
                    Dictionary<string, List<string>> i = new Dictionary<string, List<string>>();
                    i["a"] = ii;
                    i["b"] = ii;

                    Dictionary<string, List<string>> o;
                    Dictionary<string, List<string>> r;
                    (r, o) = p.opNDGSS(i);

                    foreach (string key in i.Keys)
                    {
                        test(Ice.Collections.Equals(i[key], o[key]));
                        test(Ice.Collections.Equals(i[key], r[key]));
                    }
                }
            }
        }
    }
}
