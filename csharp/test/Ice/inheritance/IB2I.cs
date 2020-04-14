//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.inheritance
{
    public sealed class IB2 : Test.MB.IIB2
    {
        public Test.MA.IIAPrx? iaop(Test.MA.IIAPrx? p, Current current) => p;

        public Test.MB.IIB2Prx? ib2op(Test.MB.IIB2Prx? p, Current current) => p;
    }
}
