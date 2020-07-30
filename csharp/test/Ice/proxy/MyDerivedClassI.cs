//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

namespace ZeroC.Ice.Test.Proxy
{
    public sealed class MyDerivedClass : IMyDerivedClass
    {
        public IObjectPrx? Echo(IObjectPrx? obj, Current c) => obj;

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        public IReadOnlyDictionary<string, string> GetContext(Current current) => _ctx!;

        public bool IceIsA(string typeId, Current current)
        {
            _ctx = current.Context;
            return typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private Dictionary<string, string>? _ctx;
    }
}
