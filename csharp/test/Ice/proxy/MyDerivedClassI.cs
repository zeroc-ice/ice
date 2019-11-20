//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

namespace Ice
{
    namespace proxy
    {
        public sealed class MyDerivedClassI : Ice.Object<Test.MyDerivedClass, Test.MyDerivedClassTraits>, Test.MyDerivedClass
        {
            public MyDerivedClassI()
            {
            }

            public Ice.ObjectPrx echo(Ice.ObjectPrx obj, Ice.Current c)
            {
                return obj;
            }

            public void shutdown(Ice.Current current)
            {
                current.adapter.GetCommunicator().shutdown();
            }

            public Dictionary<string, string> getContext(Ice.Current current)
            {
                return _ctx;
            }

            public override bool IceIsA(string s, Ice.Current current)
            {
                _ctx = current.ctx;
                Test.MyDerivedClassTraits myDerivedClassT = default;
                return myDerivedClassT.Ids.Contains(s);
            }

            private Dictionary<string, string> _ctx;
        }
    }
}
