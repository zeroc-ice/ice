//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.facets
{
    public sealed class A : Test.IA
    {
        public string callA(Current current) => "A";
    }
}
