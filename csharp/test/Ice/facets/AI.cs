//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class AI : Test.A
        {
            public AI()
            {
            }

            public string callA(Ice.Current current)
            {
                return "A";
            }
        }
    }
}
