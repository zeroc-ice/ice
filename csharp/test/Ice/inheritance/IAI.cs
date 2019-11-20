//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public sealed class IAI : Test.MA.IA
        {
            public IAI()
            {
            }

            public Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
