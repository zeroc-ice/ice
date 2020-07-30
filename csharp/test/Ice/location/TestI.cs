//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Location
{
    public class TestIntf : ITestIntf
    {
        internal TestIntf(ObjectAdapter adapter1, ObjectAdapter adapter2, ServerLocatorRegistry registry)
        {
            _adapter1 = adapter1;
            _adapter2 = adapter2;
            _registry = registry;

            _registry.AddObject(_adapter1.Add("hello", new Hello(), IObjectPrx.Factory));
        }

        public void Shutdown(Current current) => _adapter1.Communicator.ShutdownAsync();

        public IHelloPrx GetHello(Current current) =>
            _adapter1.CreateIndirectProxy("hello", IHelloPrx.Factory);

        public IHelloPrx GetReplicatedHello(Current current) =>
            _adapter1.CreateProxy("hello", IHelloPrx.Factory);

        public void MigrateHello(Current current)
        {
            var id = Identity.Parse("hello");

            IObject? servant = _adapter1.Remove(id);
            if (servant != null)
            {
                _registry.AddObject(_adapter2.Add(id, servant, IObjectPrx.Factory), current);
            }
            else
            {
                servant = _adapter2.Remove(id);
                TestHelper.Assert(servant != null);
                _registry.AddObject(_adapter1.Add(id, servant, IObjectPrx.Factory), current);
            }
        }

        private ObjectAdapter _adapter1;
        private ObjectAdapter _adapter2;
        private ServerLocatorRegistry _registry;
    }
}
