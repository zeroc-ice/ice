// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
