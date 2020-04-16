//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.inheritance
{
    public sealed class IB1 : Test.MB.IIB1
    {
        public Test.MA.IIAPrx? iaop(Test.MA.IIAPrx? p, Current current) => p;

        public Test.MB.IIB1Prx? ib1op(Test.MB.IIB1Prx? p, Current current) => p;
    }
}
