//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.location
{
    public class TestIntf : Test.ITestIntf
    {
        internal TestIntf(ObjectAdapter adapter1, ObjectAdapter adapter2, ServerLocatorRegistry registry)
        {
            _adapter1 = adapter1;
            _adapter2 = adapter2;
            _registry = registry;

            _registry.AddObject(_adapter1.Add("hello", new Hello(), IObjectPrx.Factory));
        }

        public void shutdown(Current current) => _adapter1.Communicator.Shutdown();

        public Test.IHelloPrx getHello(Current current) =>
            _adapter1.CreateIndirectProxy("hello", Test.IHelloPrx.Factory);

        public Test.IHelloPrx getReplicatedHello(Current current) =>
            _adapter1.CreateProxy("hello", Test.IHelloPrx.Factory);

        public void migrateHello(Current current)
        {
            var id = Identity.Parse("hello");

            IObject? servant = _adapter1.Remove(id);
            if (servant != null)
            {
                _registry.addObject(_adapter2.Add(id, servant, IObjectPrx.Factory), current);
            }
            else
            {
                servant = _adapter2.Remove(id);
                TestHelper.Assert(servant != null);
                _registry.addObject(_adapter1.Add(id, servant, IObjectPrx.Factory), current);
            }
        }

        private ObjectAdapter _adapter1;
        private ObjectAdapter _adapter2;
        private ServerLocatorRegistry _registry;
    }
}
