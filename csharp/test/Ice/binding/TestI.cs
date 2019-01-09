// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
