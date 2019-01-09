// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace facets
    {
        public sealed class FI : Test.FDisp_
        {
            public FI()
            {
            }

            public override string callE(Ice.Current current)
            {
                return "E";
            }

            public override string callF(Ice.Current current)
            {
                return "F";
            }
        }
    }
}
