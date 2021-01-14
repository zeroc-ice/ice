// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.DictMapping
{
    public sealed class AsyncMyClass : IAsyncMyClass
    {
        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            current.Communicator.ShutdownAsync();
            return default;
        }

        public ValueTask<(IReadOnlyDictionary<int, int>, IReadOnlyDictionary<int, int>)> OpNVAsync(
            Dictionary<int, int> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IReadOnlyDictionary<string, string>, IReadOnlyDictionary<string, string>)> OpNRAsync(
            Dictionary<string, string> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IReadOnlyDictionary<string, Dictionary<int, int>>,
                          IReadOnlyDictionary<string, Dictionary<int, int>>)> OpNDVAsync(
            Dictionary<string, Dictionary<int, int>> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IReadOnlyDictionary<string, Dictionary<string, string>>,
                          IReadOnlyDictionary<string, Dictionary<string, string>>)> OpNDRAsync(
            Dictionary<string, Dictionary<string, string>> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IReadOnlyDictionary<string, int[]>,
                          IReadOnlyDictionary<string, int[]>)> OpNDAISAsync(
            Dictionary<string, int[]> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IReadOnlyDictionary<string, List<int>>,
                          IReadOnlyDictionary<string, List<int>>)> OpNDGISAsync(
            Dictionary<string, List<int>> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IReadOnlyDictionary<string, string[]>,
                          IReadOnlyDictionary<string, string[]>)> OpNDASSAsync(
            Dictionary<string, string[]> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IReadOnlyDictionary<string, List<string>>,
                          IReadOnlyDictionary<string, List<string>>)> OpNDGSSAsync(
            Dictionary<string, List<string>> i,
            Current current,
            CancellationToken cancel) => new((i, i));
    }
}
