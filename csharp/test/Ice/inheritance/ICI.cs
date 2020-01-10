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

            public Test.MA.IIAPrx iaop(Test.MA.IIAPrx p, Ice.Current current)
            {
                return p;
            }

            public Test.MA.IICPrx icop(Test.MA.IICPrx p, Ice.Current current)
            {
                return p;
            }

            public Test.MB.IIB1Prx ib1op(Test.MB.IIB1Prx p, Ice.Current current)
            {
                return p;
            }

            public Test.MB.IIB2Prx ib2op(Test.MB.IIB2Prx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
