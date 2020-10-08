// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace ZeroC.Ice.Test.Proxy
{
    public sealed class MyDerivedClass : IMyDerivedClass
    {
        public IObjectPrx? Echo(IObjectPrx? obj, Current c, CancellationToken cancel) => obj;

        public IEnumerable<string> GetLocation(Current current, CancellationToken cancel) => current.Location;

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Adapter.Communicator.ShutdownAsync();

        public IReadOnlyDictionary<string, string> GetContext(Current current, CancellationToken cancel) => _ctx!;

        public bool IceIsA(string typeId, Current current, CancellationToken cancel)
        {
            _ctx = current.Context;
            return typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private Dictionary<string, string>? _ctx;
    }
}
