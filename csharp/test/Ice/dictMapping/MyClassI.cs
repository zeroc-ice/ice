// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;

namespace ZeroC.Ice.Test.DictMapping
{
    public sealed class MyClass : IMyClass
    {
        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public (IReadOnlyDictionary<int, int>, IReadOnlyDictionary<int, int>) OpNV(
            Dictionary<int, int> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IReadOnlyDictionary<string, string>, IReadOnlyDictionary<string, string>) OpNR(
            Dictionary<string, string> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IReadOnlyDictionary<string, Dictionary<int, int>>,
                IReadOnlyDictionary<string, Dictionary<int, int>>) OpNDV(
            Dictionary<string, Dictionary<int, int>> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IReadOnlyDictionary<string, Dictionary<string, string>>,
                IReadOnlyDictionary<string, Dictionary<string, string>>) OpNDR(
            Dictionary<string, Dictionary<string, string>> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IReadOnlyDictionary<string, int[]>, IReadOnlyDictionary<string, int[]>) OpNDAIS(
            Dictionary<string, int[]> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IReadOnlyDictionary<string, List<int>>, IReadOnlyDictionary<string, List<int>>) OpNDGIS(
            Dictionary<string, List<int>> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IReadOnlyDictionary<string, string[]>, IReadOnlyDictionary<string, string[]>) OpNDASS(
            Dictionary<string, string[]> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IReadOnlyDictionary<string, List<string>>, IReadOnlyDictionary<string, List<string>>) OpNDGSS(
            Dictionary<string, List<string>> i,
            Current current,
            CancellationToken cancel) => (i, i);
    }
}
