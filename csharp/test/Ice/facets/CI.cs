// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Facets
{
    public sealed class C : IC
    {
        public string CallA(Current current, CancellationToken cancel) => "A";

        public string CallC(Current current, CancellationToken cancel) => "C";
    }
}
