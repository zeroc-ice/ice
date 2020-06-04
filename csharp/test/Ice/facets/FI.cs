//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class F : IF
    {
        public F()
        {
        }

        public string callE(Current current) => "E";

        public string callF(Current current) => "F";
    }
}
