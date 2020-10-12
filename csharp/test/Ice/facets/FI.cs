// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Facets
{
    public sealed class F : IF
    {
        public string CallE(Current current, CancellationToken cancel) => "E";

        public string CallF(Current current, CancellationToken cancel) => "F";
    }
}
