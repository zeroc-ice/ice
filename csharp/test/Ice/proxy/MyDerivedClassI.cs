//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

namespace Ice.proxy
{
    public sealed class MyDerivedClass : Object<Test.IMyDerivedClass>, Test.IMyDerivedClass
    {

        public IObjectPrx echo(IObjectPrx obj, Current c) => obj;

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public Dictionary<string, string> getContext(Current current) => _ctx;

        public override bool IceIsA(string s, Current current)
        {
            _ctx = current.Context;
            return Ice.TypeIdAttribute.GetAllTypeIds(typeof(Test.IMyDerivedClass)).Contains(s);
        }

        private Dictionary<string, string> _ctx;
    }
}
