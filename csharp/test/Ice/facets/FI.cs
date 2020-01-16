//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class F : Test.IF
        {
            public F()
            {
            }

            public string callE(Ice.Current current)
            {
                return "E";
            }

            public string callF(Ice.Current current)
            {
                return "F";
            }
        }
    }
}
