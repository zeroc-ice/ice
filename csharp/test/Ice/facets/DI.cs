//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class D : ID
    {
        public string callA(Current current) => "A";

        public string callB(Current current) => "B";

        public string callC(Current current) => "C";

        public string callD(Current current) => "D";
    }
}
