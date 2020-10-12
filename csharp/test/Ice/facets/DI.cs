// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Facets
{
    public sealed class D : ID
    {
        public string CallA(Current current, CancellationToken cancel) => "A";

        public string CallB(Current current, CancellationToken cancel) => "B";

        public string CallC(Current current, CancellationToken cancel) => "C";

        public string CallD(Current current, CancellationToken cancel) => "D";
    }
}
