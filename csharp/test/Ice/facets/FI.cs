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

        public string CallE(Current current) => "E";

        public string CallF(Current current) => "F";
    }
}
