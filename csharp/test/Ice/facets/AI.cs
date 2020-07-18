//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class A : IA
    {
        public string CallA(Current current) => "A";
    }
}
