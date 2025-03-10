// Copyright (c) ZeroC, Inc.

namespace Ice.facets
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

