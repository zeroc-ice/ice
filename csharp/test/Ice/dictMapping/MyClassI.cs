//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace dictMapping
    {
        public sealed class MyClassI : Test.MyClass
        {
            public void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public Dictionary<int, int> opNV(Dictionary<int, int> i, out Dictionary<int, int> o,
                                                      Ice.Current current)
            {
                o = i;
                return i;
            }

            public Dictionary<string, string> opNR(Dictionary<string, string> i, out Dictionary<string, string> o,
                                                            Ice.Current current)
            {
                o = i;
                return i;
            }

            public Dictionary<string, Dictionary<int, int>> opNDV(Dictionary<string, Dictionary<int, int>> i,
                                                                           out Dictionary<string, Dictionary<int, int>> o,
                                                                           Ice.Current current)
            {
                o = i;
                return i;
            }

            public Dictionary<string, Dictionary<string, string>> opNDR(Dictionary<
                                                                       string, Dictionary<string, string>> i,
                                                                       out Dictionary<string, Dictionary<string, string>> o,
                                                                       Ice.Current current)
            {
                o = i;
                return i;
            }

            public Dictionary<string, int[]> opNDAIS(Dictionary<string, int[]> i,
                                                              out Dictionary<string, int[]> o,
                                                              Ice.Current current)
            {
                o = i;
                return i;
            }

            public Dictionary<string, List<int>> opNDGIS(Dictionary<string, List<int>> i,
                                                                  out Dictionary<string, List<int>> o,
                                                                  Ice.Current current)
            {
                o = i;
                return i;
            }

            public Dictionary<string, string[]> opNDASS(Dictionary<string, string[]> i,
                                                                 out Dictionary<string, string[]> o,
                                                                 Ice.Current current)
            {
                o = i;
                return i;
            }

            public Dictionary<string, List<string>> opNDGSS(Dictionary<string, List<string>> i,
                                                                     out Dictionary<string, List<string>> o,
                                                                     Ice.Current current)
            {
                o = i;
                return i;
            }
        }
    }
}
