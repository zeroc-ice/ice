//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class DI : Test.D
        {
            public DI()
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

            public string callC(Ice.Current current)
            {
                return "C";
            }

            public string callD(Ice.Current current)
            {
                return "D";
            }
        }
    }
}
