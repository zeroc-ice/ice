//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice.proxy.AMD
{
    public sealed class MyDerivedClass : ObjectOperations<Test.IMyDerivedClass>, Test.IMyDerivedClass
    {
        public Task<IObjectPrx> echoAsync(IObjectPrx obj, Current c) => Task.FromResult(obj);

        public Task shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return null;
        }

        public Task<Dictionary<string, string>> getContextAsync(Current current) => Task.FromResult(_ctx);

        public override bool IceIsA(string s, Current current)
        {
            _ctx = current.Context;
            return base.IceIsA(s, current);
        }

        private Dictionary<string, string> _ctx;
    }
}
