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

            public Test.MA.IIAPrx iaop(Test.MA.IIAPrx p, Ice.Current current)
            {
                return p;
            }

            public Test.MB.IIB1Prx ib1op(Test.MB.IIB1Prx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
