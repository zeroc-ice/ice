//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.location.Test;

namespace Ice.location
{
    public class TestIntf : ITestIntf
    {
        internal TestIntf(ObjectAdapter adapter1, ObjectAdapter adapter2, ServerLocatorRegistry registry)
        {
            _adapter1 = adapter1;
            _adapter2 = adapter2;
            _registry = registry;

            _registry.addObject(_adapter1.Add(new Hello(), "hello"));
        }

        public void shutdown(Current current) => _adapter1.Communicator.Shutdown();

        public IHelloPrx getHello(Current current) =>
            IHelloPrx.UncheckedCast(_adapter1.CreateIndirectProxy("hello"));

        public IHelloPrx getReplicatedHello(Current current) =>
            IHelloPrx.UncheckedCast(_adapter1.CreateProxy("hello"));

        public void migrateHello(Current current)
        {
            var id = Identity.Parse("hello");
            try
            {
                _registry.addObject(_adapter2.Add(_adapter1.Remove(id), id), current);
            }
            catch (NotRegisteredException)
            {
                _registry.addObject(_adapter1.Add(_adapter2.Remove(id), id), current);
            }
        }

        private ObjectAdapter _adapter1;
        private ObjectAdapter _adapter2;
        private ServerLocatorRegistry _registry;
    }
}
