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
        public ValueTask<IObjectPrx?> echoAsync(IObjectPrx? obj, Current c)
            => new ValueTask<IObjectPrx?>(obj);

        public ValueTask shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<Dictionary<string, string>> getContextAsync(Current current)
            => new ValueTask<Dictionary<string, string>>(_ctx!);

        public bool IceIsA(string typeId, Current current)
        {
            _ctx = current.Context;
            return typeof(Test.IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private Dictionary<string, string>? _ctx;
    }
}
