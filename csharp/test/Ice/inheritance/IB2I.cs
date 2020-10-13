// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class B2 : MB.IB2
    {
        public MA.IAPrx? Iaop(MA.IAPrx? p, Current current, CancellationToken cancel) => p;

        public MB.IB2Prx? Ib2op(MB.IB2Prx? p, Current current, CancellationToken cancel) => p;
    }
}
