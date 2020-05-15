//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.facets
{
    public sealed class D : Test.ID
    {
        public string callA(Current current) => "A";

        public string callB(Current current) => "B";

        public string callC(Current current) => "C";

        public string callD(Current current) => "D";
    }
}
