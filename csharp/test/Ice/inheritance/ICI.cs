// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class C : MA.IC
    {
        public MA.IAPrx? Iaop(MA.IAPrx? p, Current current, CancellationToken cancel) => p;

        public MA.ICPrx? Icop(MA.ICPrx? p, Current current, CancellationToken cancel) => p;

        public MB.IB1Prx? Ib1op(MB.IB1Prx? p, Current current, CancellationToken cancel) => p;

        public MB.IB2Prx? Ib2op(MB.IB2Prx? p, Current current, CancellationToken cancel) => p;
    }
}
