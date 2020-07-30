//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.IceGrid.Test.Simple
{
    public sealed class TestIntf : ITestIntf
    {
        public void Shutdown(Ice.Current current) => current.Adapter.Communicator.ShutdownAsync();
    }
}
