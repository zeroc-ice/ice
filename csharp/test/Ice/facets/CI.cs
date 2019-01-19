//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class CI : Test.CDisp_
        {
            public CI()
            {
            }

            public override string callA(Ice.Current current)
            {
                return "A";
            }

            public override string callC(Ice.Current current)
            {
                return "C";
            }
        }
    }
}
