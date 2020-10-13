// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Proxy
{
    public sealed class AsyncMyDerivedClass : IAsyncMyDerivedClass
    {
        public ValueTask<IObjectPrx?> EchoAsync(IObjectPrx? obj, Current c, CancellationToken cancel) =>
            new ValueTask<IObjectPrx?>(obj);

        public ValueTask<IEnumerable<string>> GetLocationAsync(Current current, CancellationToken cancel) =>
            new ValueTask<IEnumerable<string>>(current.Location);

        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            current.Adapter.Communicator.ShutdownAsync();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<IReadOnlyDictionary<string, string>> GetContextAsync(
            Current current,
            CancellationToken cancel) =>
            new ValueTask<IReadOnlyDictionary<string, string>>(_ctx!);

        public bool IceIsA(string typeId, Current current, CancellationToken cancel)
        {
            _ctx = current.Context;
            return typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private IReadOnlyDictionary<string, string>? _ctx;
    }
}
