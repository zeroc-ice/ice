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
            _ia = adapter.Add(new IA());
            _ib1 = adapter.Add(new IB1());
            _ib2 = adapter.Add(new IB2());
            _ic = adapter.Add(new IC());
        }

        public IIAPrx iaop(Current current) => _ia;
        public IIB1Prx ib1op(Current current) => _ib1;

        public IIB2Prx ib2op(Current current) => _ib2;

        public IICPrx icop(Current current) => _ic;

        public void shutdown(Current current) => current.Adapter.Communicator.shutdown();

        private IIAPrx _ia;
        private IIB1Prx _ib1;
        private IIB2Prx _ib2;
        private IICPrx _ic;
    }
}
