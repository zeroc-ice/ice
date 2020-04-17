//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Linq;

namespace Ice.proxy
{
    public sealed class MyDerivedClass : Test.IMyDerivedClass
    {

        public IObjectPrx? echo(IObjectPrx? obj, Current c) => obj;

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public IReadOnlyDictionary<string, string> getContext(Current current) => _ctx!;

        public bool IceIsA(string typeId, Current current)
        {
            _ctx = current.Context;
            return typeof(Test.IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private Dictionary<string, string>? _ctx;
    }
}
