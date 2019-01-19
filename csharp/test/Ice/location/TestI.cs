//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace location
    {
        public class TestI : Test.TestIntfDisp_
        {
            internal TestI(Ice.ObjectAdapter adapter1,
                           Ice.ObjectAdapter adapter2, ServerLocatorRegistry registry)
            {
                _adapter1 = adapter1;
                _adapter2 = adapter2;
                _registry = registry;

                _registry.addObject(_adapter1.add(new HelloI(), Ice.Util.stringToIdentity("hello")));
            }

            public override void shutdown(Ice.Current current)
            {
                _adapter1.getCommunicator().shutdown();
            }

            public override Test.HelloPrx getHello(Ice.Current current)
            {
                return Test.HelloPrxHelper.uncheckedCast(_adapter1.createIndirectProxy(
                                                                Ice.Util.stringToIdentity("hello")));
            }

            public override Test.HelloPrx getReplicatedHello(Ice.Current current)
            {
                return Test.HelloPrxHelper.uncheckedCast(_adapter1.createProxy(
                                                                Ice.Util.stringToIdentity("hello")));
            }

            public override void migrateHello(Ice.Current current)
            {
                Ice.Identity id = Ice.Util.stringToIdentity("hello");
                try
                {
                    _registry.addObject(_adapter2.add(_adapter1.remove(id), id));
                }
                catch(Ice.NotRegisteredException)
                {
                    _registry.addObject(_adapter1.add(_adapter2.remove(id), id));
                }
            }

            private Ice.ObjectAdapter _adapter1;
            private Ice.ObjectAdapter _adapter2;
            private ServerLocatorRegistry _registry;
        }
    }
}
