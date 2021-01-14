// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Proxy
{
    internal sealed class AsyncMyDerivedClass : IAsyncMyDerivedClass
    {
        private IReadOnlyDictionary<string, string>? _ctx;

        public ValueTask<IObjectPrx?> EchoAsync(IObjectPrx? obj, Current c, CancellationToken cancel) =>
            new(obj);

        public ValueTask<IEnumerable<string>> GetLocationAsync(Current current, CancellationToken cancel) =>
            new(current.Location);

        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            current.Communicator.ShutdownAsync();
            return default;
        }

        public ValueTask<IReadOnlyDictionary<string, string>> GetContextAsync(
            Current current,
            CancellationToken cancel) =>
            new(_ctx!);

        public ValueTask<bool> IceIsAAsync(string typeId, Current current, CancellationToken cancel)
        {
            _ctx = current.Context;
            return new(typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(typeId));
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
