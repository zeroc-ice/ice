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
                _ia = Test.MA.IAPrxHelper.uncheckedCast(adapter.Add(new IAI()));
                _ib1 = Test.MB.IB1PrxHelper.uncheckedCast(adapter.Add(new IB1I()));
                _ib2 = Test.MB.IB2PrxHelper.uncheckedCast(adapter.Add(new IB2I()));
                _ic = Test.MA.ICPrxHelper.uncheckedCast(adapter.Add(new ICI()));
            }

            public Test.MA.IAPrx iaop(Ice.Current current)
            {
                return _ia;
            }

            public Test.MB.IB1Prx ib1op(Ice.Current current)
            {
                return _ib1;
            }

            public Test.MB.IB2Prx ib2op(Ice.Current current)
            {
                return _ib2;
            }

            public Test.MA.ICPrx icop(Ice.Current current)
            {
                return _ic;
            }

            public void shutdown(Ice.Current current)
            {
                current.adapter.GetCommunicator().shutdown();
            }

            private Test.MA.IAPrx _ia;
            private Test.MB.IB1Prx _ib1;
            private Test.MB.IB2Prx _ib2;
            private Test.MA.ICPrx _ic;
        }
    }
}
