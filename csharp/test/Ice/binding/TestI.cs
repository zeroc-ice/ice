//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.binding
{
    public class TestIntf : Test.ITestIntf
    {
        public string getAdapterName(Current current) => current.Adapter.GetName();
    }
}
