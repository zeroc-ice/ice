//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class EI : Test.EDisp_
        {
            public EI()
            {
            }

            public override string callE(Ice.Current current)
            {
                return "E";
            }
        }
    }
}
