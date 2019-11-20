//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public sealed class IB2I : Test.MB.IB2
        {
            public IB2I()
            {
            }

            public Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
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
