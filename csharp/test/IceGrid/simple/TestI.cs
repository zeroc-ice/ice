//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.IceGrid.Test.Simple
{
    public sealed class TestIntf : ITestIntf
    {
        public void shutdown(ZeroC.Ice.Current current) => current.Adapter.Communicator.Shutdown();
    }
}
