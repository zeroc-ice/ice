//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class IB2 : MB.IIB2
    {
        public MA.IIAPrx? Iaop(MA.IIAPrx? p, Current current) => p;

        public MB.IIB2Prx? Ib2op(MB.IIB2Prx? p, Current current) => p;
    }
}
