// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class A : MA.IA
    {
        public MA.IAPrx? Iaop(MA.IAPrx? p, Current current, CancellationToken cancel) => p;
    }
}
