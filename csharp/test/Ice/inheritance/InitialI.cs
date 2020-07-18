//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice.Test.Inheritance.MA;
using ZeroC.Ice.Test.Inheritance.MB;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class InitialI : IInitial
    {
        public InitialI(ObjectAdapter adapter)
        {
            _ia = adapter.AddWithUUID(new IA(), IIAPrx.Factory);
            _ib1 = adapter.AddWithUUID(new IB1(), IIB1Prx.Factory);
            _ib2 = adapter.AddWithUUID(new IB2(), IIB2Prx.Factory);
            _ic = adapter.AddWithUUID(new IC(), IICPrx.Factory);
        }

        public IIAPrx Iaop(Current current) => _ia;
        public IIB1Prx Ib1op(Current current) => _ib1;

        public IIB2Prx Ib2op(Current current) => _ib2;

        public IICPrx Icop(Current current) => _ic;

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        private readonly IIAPrx _ia;
        private readonly IIB1Prx _ib1;
        private readonly IIB2Prx _ib2;
        private readonly IICPrx _ic;
    }
}
