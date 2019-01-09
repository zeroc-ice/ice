// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace facets
    {
        public sealed class AI : Test.ADisp_
        {
            public AI()
            {
            }

            public override string callA(Ice.Current current)
            {
                return "A";
            }
        }
    }
}
