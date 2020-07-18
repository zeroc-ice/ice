//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Binding
{
    public class TestIntf : ITestIntf
    {
        public string GetAdapterName(Current current) => current.Adapter.Name;
    }
}
