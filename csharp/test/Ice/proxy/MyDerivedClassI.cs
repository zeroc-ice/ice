// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Test;

namespace ZeroC.Ice.Test.Proxy
{
    internal sealed class MyDerivedClass : IMyDerivedClass
    {
        private SortedDictionary<string, string>? _ctx;

        public IObjectPrx? Echo(IObjectPrx? obj, Current c, CancellationToken cancel) => obj;

        public IEnumerable<string> GetLocation(Current current, CancellationToken cancel) => current.Location;

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public IReadOnlyDictionary<string, string> GetContext(Current current, CancellationToken cancel) => _ctx!;

        public ValueTask<bool> IceIsAAsync(string typeId, Current current, CancellationToken cancel)
        {
            _ctx = current.Context;
            return new(typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(typeId));
        }

        public IRelativeTestPrx OpRelative(ICallbackPrx callback, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(callback.IsFixed);
            IRelativeTestPrx relativeTest =
                current.Adapter.AddWithUUID(new RelativeTest(), IRelativeTestPrx.Factory).Clone(relative: true);

            TestHelper.Assert(callback.Op(relativeTest, cancel: cancel) == 1);
            return relativeTest;
        }
    }

    internal sealed class RelativeTest : IRelativeTest
    {
        private int _count;

        public int DoIt(Current current, CancellationToken cancel) => ++_count;
    }
}
