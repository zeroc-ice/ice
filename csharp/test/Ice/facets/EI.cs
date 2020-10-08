// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Facets
{
    public sealed class E : IE
    {
        public string CallE(Current current, CancellationToken cancel) => "E";
    }
}
