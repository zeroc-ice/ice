//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.AMD.Proxy
{
    public sealed class MyDerivedClass : IMyDerivedClass
    {
        public ValueTask<IObjectPrx?> echoAsync(IObjectPrx? obj, Current c)
            => new ValueTask<IObjectPrx?>(obj);

        public ValueTask shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<IReadOnlyDictionary<string, string>> getContextAsync(Current current)
            => new ValueTask<IReadOnlyDictionary<string, string>>(_ctx!);

        public bool IceIsA(string typeId, Current current)
        {
            _ctx = current.Context;
            return typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private IReadOnlyDictionary<string, string>? _ctx;
    }
}
