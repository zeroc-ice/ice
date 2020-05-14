//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.facets
{
    public sealed class F : Test.IF
    {
        public F()
        {
        }

        public string callE(Current current) => "E";

        public string callF(Current current) => "F";
    }
}
