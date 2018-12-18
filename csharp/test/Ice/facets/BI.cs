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
        public sealed class BI : Test.BDisp_
        {
            public BI()
            {
            }

            public override string callA(Ice.Current current)
            {
                return "A";
            }

            public override string callB(Ice.Current current)
            {
                return "B";
            }
        }
    }
}
