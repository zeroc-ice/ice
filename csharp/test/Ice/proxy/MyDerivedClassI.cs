//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

namespace Ice.proxy
{
    public sealed class MyDerivedClass : Object<Test.IMyDerivedClass, Test.MyDerivedClassTraits>, Test.IMyDerivedClass
    {

        public IObjectPrx echo(IObjectPrx obj, Current c) => obj;

        public void shutdown(Current current) => current.Adapter.Communicator.shutdown();

        public Dictionary<string, string> getContext(Current current) => _ctx;

        public override bool IceIsA(string s, Current current)
        {
            _ctx = current.Context;
            Test.MyDerivedClassTraits myDerivedClassT = default;
            return myDerivedClassT.Ids.Contains(s);
        }

        private Dictionary<string, string> _ctx;
    }
}
