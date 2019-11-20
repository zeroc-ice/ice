//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class EI : Test.E
        {
            public EI()
            {
            }

            public string callE(Ice.Current current)
            {
                return "E";
            }
        }
    }
}
