//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Proxy
{
    public sealed class MyDerivedClassAsync : IMyDerivedClassAsync
    {
        public ValueTask<IObjectPrx?> EchoAsync(IObjectPrx? obj, Current c) =>
            new ValueTask<IObjectPrx?>(obj);

        public ValueTask ShutdownAsync(Current current)
        {
            current.Adapter.Communicator.ShutdownAsync();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<IReadOnlyDictionary<string, string>> GetContextAsync(Current current) =>
            new ValueTask<IReadOnlyDictionary<string, string>>(_ctx!);

        public bool IceIsA(string typeId, Current current)
        {
            _ctx = current.Context;
            return typeof(IMyDerivedClass).GetAllIceTypeIds().Contains(typeId);
        }

        private IReadOnlyDictionary<string, string>? _ctx;
    }
}
