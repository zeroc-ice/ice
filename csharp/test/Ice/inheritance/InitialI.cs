// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using ZeroC.Ice.Test.Inheritance.MA;
using ZeroC.Ice.Test.Inheritance.MB;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class InitialI : IInitial
    {
        private readonly IAPrx _ia;
        private readonly IB1Prx _ib1;
        private readonly IB2Prx _ib2;
        private readonly ICPrx _ic;

        public InitialI(ObjectAdapter adapter)
        {
            _ia = adapter.AddWithUUID(new A(), IAPrx.Factory);
            _ib1 = adapter.AddWithUUID(new B1(), IB1Prx.Factory);
            _ib2 = adapter.AddWithUUID(new B2(), IB2Prx.Factory);
            _ic = adapter.AddWithUUID(new C(), ICPrx.Factory);
        }

        public IAPrx Iaop(Current current, CancellationToken cancel) => _ia;
        public IB1Prx Ib1op(Current current, CancellationToken cancel) => _ib1;

        public IB2Prx Ib2op(Current current, CancellationToken cancel) => _ib2;

        public ICPrx Icop(Current current, CancellationToken cancel) => _ic;

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();
    }
}
