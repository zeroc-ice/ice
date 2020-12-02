// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

using Test;

namespace ZeroC.Ice.Test.Proxy
{
    internal sealed class AsyncMyDerivedClass : IAsyncMyDerivedClass
    {
        private IReadOnlyDictionary<string, string>? _ctx;

        public ValueTask<IObjectPrx?> EchoAsync(IObjectPrx? obj, Current c, CancellationToken cancel) =>
            new ValueTask<IObjectPrx?>(obj);

        public ValueTask<IEnumerable<string>> GetLocationAsync(Current current, CancellationToken cancel) =>
            new ValueTask<IEnumerable<string>>(current.Location);

        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            current.Communicator.ShutdownAsync();
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

        public async ValueTask<IRelativeTestPrx> OpRelativeAsync(
            ICallbackPrx callback,
            Current current,
            CancellationToken cancel)
        {
            TestHelper.Assert(callback.IsFixed);
            IRelativeTestPrx relativeTest =
                current.Adapter.AddWithUUID(new RelativeTest(), IRelativeTestPrx.Factory).Clone(relative: true);

            TestHelper.Assert(await callback.OpAsync(relativeTest, cancel: cancel) == 1);
            return relativeTest;
        }
    }

    internal sealed class RelativeTest : IRelativeTest
    {
        private int _count;

        public int DoIt(Current current, CancellationToken cancel) => ++_count;
    }
}
