//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace binding
    {
        public class TestI : Test.TestIntf
        {
            public string getAdapterName(Ice.Current current)
            {
                return current.Adapter.GetName();
            }
        }
    }
}
