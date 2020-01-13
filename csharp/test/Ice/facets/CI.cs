//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.facets
{
    public sealed class C : Test.IC
    {
        public string callA(Current current) => "A";

        public string callC(Current current) => "C";
    }
}
