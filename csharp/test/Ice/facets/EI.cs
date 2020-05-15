//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.facets
{
    public sealed class E : Test.IE
    {
        public E()
        {
        }

        public string callE(Current current) => "E";
    }
}
