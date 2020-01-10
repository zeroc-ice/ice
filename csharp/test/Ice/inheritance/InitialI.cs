//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.inheritance.Test;
using Ice.inheritance.Test.MA;
using Ice.inheritance.Test.MB;

namespace Ice
{
    namespace inheritance
    {
        public sealed class InitialI : Test.Initial
        {
            public InitialI(Ice.ObjectAdapter adapter)
            {
                _ia = adapter.Add(new IAI());
                _ib1 = adapter.Add(new IB1I());
                _ib2 = adapter.Add(new IB2I());
                _ic = adapter.Add(new ICI());
            }

            public Test.MA.IIAPrx iaop(Ice.Current current)
            {
                return _ia;
            }

            public Test.MB.IIB1Prx ib1op(Ice.Current current)
            {
                return _ib1;
            }

            public Test.MB.IIB2Prx ib2op(Ice.Current current)
            {
                return _ib2;
            }

            public Test.MA.IICPrx icop(Ice.Current current)
            {
                return _ic;
            }

            public void shutdown(Ice.Current current)
            {
                current.Adapter.Communicator.shutdown();
            }

            private Test.MA.IIAPrx _ia;
            private Test.MB.IIB1Prx _ib1;
            private Test.MB.IIB2Prx _ib2;
            private Test.MA.IICPrx _ic;
        }
    }
}
