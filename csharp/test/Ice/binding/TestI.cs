//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.binding
{
    public class TestIntf : Test.ITestIntf
    {
        public string getAdapterName(Current current) => current.Adapter.Name;
    }
}
