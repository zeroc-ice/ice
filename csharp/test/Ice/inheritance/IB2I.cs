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

            public Test.MA.IIAPrx iaop(Test.MA.IIAPrx p, Ice.Current current)
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
