//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class C : IC
    {
        public string CallA(Current current) => "A";

        public string CallC(Current current) => "C";
    }
}
