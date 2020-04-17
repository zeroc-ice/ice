//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.dictMapping
{
    public sealed class MyClass : Test.IMyClass
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public (IReadOnlyDictionary<int, int>, IReadOnlyDictionary<int, int>)
        opNV(Dictionary<int, int> i, Current current) => (i, i);

        public (IReadOnlyDictionary<string, string>, IReadOnlyDictionary<string, string>)
        opNR(Dictionary<string, string> i, Current current) => (i, i);

        public (IReadOnlyDictionary<string, Dictionary<int, int>>, IReadOnlyDictionary<string, Dictionary<int, int>>)
        opNDV(Dictionary<string, Dictionary<int, int>> i, Current current) => (i, i);

        public (IReadOnlyDictionary<string, Dictionary<string, string>>, IReadOnlyDictionary<string, Dictionary<string, string>>)
        opNDR(Dictionary<string, Dictionary<string, string>> i, Current current) => (i, i);

        public (IReadOnlyDictionary<string, int[]>, IReadOnlyDictionary<string, int[]>)
        opNDAIS(Dictionary<string, int[]> i, Current current) => (i, i);

        public (IReadOnlyDictionary<string, List<int>>, IReadOnlyDictionary<string, List<int>>)
        opNDGIS(Dictionary<string, List<int>> i, Current current) => (i, i);

        public (IReadOnlyDictionary<string, string[]>, IReadOnlyDictionary<string, string[]>)
        opNDASS(Dictionary<string, string[]> i, Current current) => (i, i);

        public (IReadOnlyDictionary<string, List<string>>, IReadOnlyDictionary<string, List<string>>)
        opNDGSS(Dictionary<string, List<string>> i, Current current) => (i, i);
    }
}
