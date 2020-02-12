//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace Ice.proxy.AMD
{
    public sealed class MyDerivedClass : Test.IMyDerivedClass
    {
        public Task<IObjectPrx> echoAsync(IObjectPrx obj, Current c) => Task.FromResult(obj);

        public Task? shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return null;
        }

        public Task<Dictionary<string, string>> getContextAsync(Current current) => Task.FromResult(_ctx);

        public bool IceIsA(string typeId, Current current)
        {
            _ctx = current.Context;
            return typeof(Test.IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private Dictionary<string, string> _ctx;
    }
}
