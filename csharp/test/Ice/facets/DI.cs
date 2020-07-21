//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class D : ID
    {
        public string CallA(Current current) => "A";

        public string CallB(Current current) => "B";

        public string CallC(Current current) => "C";

        public string CallD(Current current) => "D";
    }
}
