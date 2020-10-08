// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class IB2 : MB.IIB2
    {
        public MA.IIAPrx? Iaop(MA.IIAPrx? p, Current current, CancellationToken cancel) => p;

        public MB.IIB2Prx? Ib2op(MB.IIB2Prx? p, Current current, CancellationToken cancel) => p;
    }
}
