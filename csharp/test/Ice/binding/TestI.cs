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
    namespace binding
    {
        public class TestI : Test.TestIntfDisp_
        {
            public override string getAdapterName(Ice.Current current)
            {
                return current.adapter.getName();
            }
        }
    }
}
