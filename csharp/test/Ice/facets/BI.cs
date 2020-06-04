//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class B : IB
    {
        public string callA(Current current) => "A";

        public string callB(Current current) => "B";
    }
}
