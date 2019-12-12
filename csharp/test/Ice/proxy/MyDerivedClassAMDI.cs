//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice
{
    namespace proxy
    {
        namespace AMD
        {
            public sealed class MyDerivedClassI : Object<Test.MyDerivedClass, Test.MyDerivedClassTraits>, Test.MyDerivedClass
            {

                public Task<Ice.IObjectPrx> echoAsync(Ice.IObjectPrx obj, Ice.Current c)
                {
                    return Task.FromResult(obj);
                }

                public Task shutdownAsync(Ice.Current current)
                {
                    current.Adapter.Communicator.shutdown();
                    return null;
                }

                public Task<Dictionary<string, string>> getContextAsync(Ice.Current current)
                {
                    return Task.FromResult(_ctx);
                }

                public override bool IceIsA(string s, Ice.Current current)
                {
                    _ctx = current.Context;
                    return base.IceIsA(s, current);
                }

                private Dictionary<string, string> _ctx;
            }
        }
    }
}
