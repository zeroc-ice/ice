//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class IC : MA.IIC
    {
        public MA.IIAPrx? iaop(MA.IIAPrx? p, Current current) => p;

        public MA.IICPrx? icop(MA.IICPrx? p, Current current) => p;

        public MB.IIB1Prx? ib1op(MB.IIB1Prx? p, Current current) => p;

        public MB.IIB2Prx? ib2op(MB.IIB2Prx? p, Current current) => p;
    }
}
