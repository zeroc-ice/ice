//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class B : IB
    {
        public string CallA(Current current) => "A";

        public string CallB(Current current) => "B";
    }
}
