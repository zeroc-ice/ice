//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public sealed class InitialI : Test.InitialDisp_
        {
            public InitialI(Ice.ObjectAdapter adapter)
            {
                _ia = Test.MA.IAPrxHelper.uncheckedCast(adapter.addWithUUID(new IAI()));
                _ib1 = Test.MB.IB1PrxHelper.uncheckedCast(adapter.addWithUUID(new IB1I()));
                _ib2 = Test.MB.IB2PrxHelper.uncheckedCast(adapter.addWithUUID(new IB2I()));
                _ic = Test.MA.ICPrxHelper.uncheckedCast(adapter.addWithUUID(new ICI()));
            }

            public override Test.MA.IAPrx iaop(Ice.Current current)
            {
                return _ia;
            }

            public override Test.MB.IB1Prx ib1op(Ice.Current current)
            {
                return _ib1;
            }

            public override Test.MB.IB2Prx ib2op(Ice.Current current)
            {
                return _ib2;
            }

            public override Test.MA.ICPrx icop(Ice.Current current)
            {
                return _ic;
            }

            public override void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            private Test.MA.IAPrx _ia;
            private Test.MB.IB1Prx _ib1;
            private Test.MB.IB2Prx _ib2;
            private Test.MA.ICPrx _ic;
        }
    }
}
