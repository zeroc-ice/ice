// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Binding
{
    public class TestIntf : ITestIntf
    {
        public string GetAdapterName(Current current, CancellationToken cancel) => current.Adapter.Name;
    }
}
