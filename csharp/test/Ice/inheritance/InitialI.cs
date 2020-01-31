//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.inheritance.Test;
using Ice.inheritance.Test.MA;
using Ice.inheritance.Test.MB;

namespace Ice.inheritance
{
    public sealed class InitialI : Test.IInitial
    {
        public InitialI(Ice.ObjectAdapter adapter)
        {
            _ia = adapter.Add(new IA(),IIAPrx.Factory);
            _ib1 = adapter.Add(new IB1(), IIB1Prx.Factory);
            _ib2 = adapter.Add(new IB2(), IIB2Prx.Factory);
            _ic = adapter.Add(new IC(), IICPrx.Factory);
        }

        public IIAPrx iaop(Current current) => _ia;
        public IIB1Prx ib1op(Current current) => _ib1;

        public IIB2Prx ib2op(Current current) => _ib2;

        public IICPrx icop(Current current) => _ic;

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        private IIAPrx _ia;
        private IIB1Prx _ib1;
        private IIB2Prx _ib2;
        private IICPrx _ic;
    }
}
