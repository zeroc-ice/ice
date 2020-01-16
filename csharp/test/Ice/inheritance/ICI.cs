//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.inheritance
{
    public sealed class IC : Test.MA.IIC
    {
        public Test.MA.IIAPrx iaop(Test.MA.IIAPrx p, Current current) => p;

        public Test.MA.IICPrx icop(Test.MA.IICPrx p, Ice.Current current) => p;

        public Test.MB.IIB1Prx ib1op(Test.MB.IIB1Prx p, Ice.Current current) => p;

        public Test.MB.IIB2Prx ib2op(Test.MB.IIB2Prx p, Current current) => p;
    }
}
