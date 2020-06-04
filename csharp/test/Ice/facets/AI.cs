//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class A : IA
    {
        public string callA(Current current) => "A";
    }
}
