// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class IC : MA.IIC
    {
        public MA.IIAPrx? Iaop(MA.IIAPrx? p, Current current, CancellationToken cancel) => p;

        public MA.IICPrx? Icop(MA.IICPrx? p, Current current, CancellationToken cancel) => p;

        public MB.IIB1Prx? Ib1op(MB.IIB1Prx? p, Current current, CancellationToken cancel) => p;

        public MB.IIB2Prx? Ib2op(MB.IIB2Prx? p, Current current, CancellationToken cancel) => p;
    }
}
