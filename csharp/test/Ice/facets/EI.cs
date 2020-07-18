//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class E : IE
    {
        public E()
        {
        }

        public string CallE(Current current) => "E";
    }
}
