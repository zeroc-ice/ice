//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.inheritance.Test.MA;

namespace Ice
{
    namespace inheritance
    {
        public sealed class IAI : Test.MA.IA
        {
            public IAI()
            {
            }

            public Test.MA.IIAPrx iaop(Test.MA.IIAPrx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
