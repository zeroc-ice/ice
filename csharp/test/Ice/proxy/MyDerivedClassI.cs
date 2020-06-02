//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

namespace ZeroC.Ice.Test.Proxy
{
    public sealed class MyDerivedClass : IMyDerivedClass
    {
        public IObjectPrx? echo(IObjectPrx? obj, Current c) => obj;

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public IReadOnlyDictionary<string, string> getContext(Current current) => _ctx!;

        public bool IceIsA(string typeId, Current current)
        {
            _ctx = current.Context;
            return typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private Dictionary<string, string>? _ctx;
    }
}
