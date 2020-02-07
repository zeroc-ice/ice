//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.dictMapping
{
    public sealed class MyClass : Test.IMyClass
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public (Dictionary<int, int>, Dictionary<int, int>)
        opNV(Dictionary<int, int> i, Current current) => (i, i);

        public (Dictionary<string, string>, Dictionary<string, string>)
        opNR(Dictionary<string, string> i, Current current) => (i, i);

        public (Dictionary<string, Dictionary<int, int>>, Dictionary<string, Dictionary<int, int>>)
        opNDV(Dictionary<string, Dictionary<int, int>> i, Current current) => (i, i);

        public (Dictionary<string, Dictionary<string, string>>, Dictionary<string, Dictionary<string, string>>)
        opNDR(Dictionary<string, Dictionary<string, string>> i, Current current) => (i, i);

        public (Dictionary<string, int[]>, Dictionary<string, int[]>)
        opNDAIS(Dictionary<string, int[]> i, Current current) => (i, i);

        public (Dictionary<string, List<int>>, Dictionary<string, List<int>>)
        opNDGIS(Dictionary<string, List<int>> i, Current current) => (i, i);

        public (Dictionary<string, string[]>, Dictionary<string, string[]>)
        opNDASS(Dictionary<string, string[]> i, Current current) => (i, i);

        public (Dictionary<string, List<string>>, Dictionary<string, List<string>>)
        opNDGSS(Dictionary<string, List<string>> i, Current current) => (i, i);
    }
}
