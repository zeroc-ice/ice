//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class BI : Test.B
        {
            public BI()
            {
            }

            public string callA(Ice.Current current)
            {
                return "A";
            }

            public string callB(Ice.Current current)
            {
                return "B";
            }
        }
    }
}
