//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.facets
{
    public sealed class A : Test.IA
    {
        public string callA(Current current) => "A";
    }
}
