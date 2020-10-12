// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class IB1 : MB.IIB1
    {
        public MA.IIAPrx? Iaop(MA.IIAPrx? p, Current current, CancellationToken cancel) => p;

        public MB.IIB1Prx? Ib1op(MB.IIB1Prx? p, Current current, CancellationToken cancel) => p;
    }
}
