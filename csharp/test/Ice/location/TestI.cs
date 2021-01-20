// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Immutable;
using System.Threading;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Location
{
    public class TestIntf : ITestIntf
    {
        private ObjectAdapter _adapter1;
        private ObjectAdapter _adapter2;
        private ServerLocatorRegistry _registry;

        internal TestIntf(ObjectAdapter adapter1, ObjectAdapter adapter2, ServerLocatorRegistry registry)
        {
            _adapter1 = adapter1;
            _adapter2 = adapter2;
            _registry = registry;

            _registry.AddObject(_adapter1.Add("hello", new Hello(), IObjectPrx.Factory));
            _registry.AddObject(_adapter1.Add("bonjour#abc", new Hello(), IObjectPrx.Factory));
        }

        public void Shutdown(Current current, CancellationToken cancel) => _adapter1.Communicator.ShutdownAsync();

        public IHelloPrx GetHello(Current current, CancellationToken cancel) =>
            _adapter1.CreateProxy("hello", IHelloPrx.Factory).Clone(
                location: ImmutableArray.Create(_adapter1.AdapterId));

        public IHelloPrx GetReplicatedHello(Current current, CancellationToken cancel) =>
            _adapter1.CreateProxy("hello", IHelloPrx.Factory);

        public void MigrateHello(Current current, CancellationToken cancel)
        {
            var id = Identity.Parse("hello");

            IObject? servant = _adapter1.Remove(id);
            if (servant != null)
            {
                _registry.AddObject(_adapter2.Add(id, servant, IObjectPrx.Factory), current, cancel);
            }
            else
            {
                servant = _adapter2.Remove(id);
                TestHelper.Assert(servant != null);
                _registry.AddObject(_adapter1.Add(id, servant, IObjectPrx.Factory), current, cancel);
            }
        }
    }
}
