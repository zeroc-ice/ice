// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Facets
{
    public sealed class B : IB
    {
        public string CallA(Current current, CancellationToken cancel) => "A";

        public string CallB(Current current, CancellationToken cancel) => "B";
    }
}
