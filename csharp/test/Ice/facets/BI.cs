//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.facets
{
    public sealed class B : Test.IB
    {
        public string callA(Current current) => "A";

        public string callB(Current current) => "B";
    }
}
