// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class B1 : MB.IB1
    {
        public MA.IAPrx? Iaop(MA.IAPrx? p, Current current, CancellationToken cancel) => p;

        public MB.IB1Prx? Ib1op(MB.IB1Prx? p, Current current, CancellationToken cancel) => p;
    }
}
