//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class C : IC
    {
        public string callA(Current current) => "A";

        public string callC(Current current) => "C";
    }
}
