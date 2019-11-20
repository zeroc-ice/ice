//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class CI : Test.C
        {
            public CI()
            {
            }

            public string callA(Ice.Current current)
            {
                return "A";
            }

            public string callC(Ice.Current current)
            {
                return "C";
            }
        }
    }
}
