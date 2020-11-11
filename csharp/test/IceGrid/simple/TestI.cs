// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.IceGrid.Test.Simple
{
    public sealed class TestIntf : ITestIntf
    {
        public void Shutdown(Ice.Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();
    }
}
