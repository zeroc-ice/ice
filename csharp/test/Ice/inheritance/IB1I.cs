//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public sealed class IB1I : Test.MB.IB1
        {
            public IB1I()
            {
            }

            public Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
            {
                return p;
            }

            public Test.MB.IB1Prx ib1op(Test.MB.IB1Prx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
