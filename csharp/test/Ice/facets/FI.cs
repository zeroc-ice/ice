//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class FI : Test.F
        {
            public FI()
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
