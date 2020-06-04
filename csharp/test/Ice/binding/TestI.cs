//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Binding
{
    public class TestIntf : ITestIntf
    {
        public string getAdapterName(Current current) => current.Adapter.Name;
    }
}
