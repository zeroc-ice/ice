// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
