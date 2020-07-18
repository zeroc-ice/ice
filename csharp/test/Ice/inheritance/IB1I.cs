//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class IB1 : MB.IIB1
    {
        public MA.IIAPrx? Iaop(MA.IIAPrx? p, Current current) => p;

        public MB.IIB1Prx? Ib1op(MB.IIB1Prx? p, Current current) => p;
    }
}
