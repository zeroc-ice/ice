//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public sealed class ICI : Test.MA.IC
        {
            public ICI()
            {
            }

            public Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
            {
                return p;
            }

            public Test.MA.ICPrx icop(Test.MA.ICPrx p, Ice.Current current)
            {
                return p;
            }

            public Test.MB.IB1Prx ib1op(Test.MB.IB1Prx p, Ice.Current current)
            {
                return p;
            }

            public Test.MB.IB2Prx ib2op(Test.MB.IB2Prx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
