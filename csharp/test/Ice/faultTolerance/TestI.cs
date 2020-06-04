//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public sealed class TestIntf : ITestIntf
    {
        public void abort(Current current) => Process.GetCurrentProcess().Kill();

        public void idempotentAbort(Current current) => Process.GetCurrentProcess().Kill();

        public int pid(Current current) => Process.GetCurrentProcess().Id;

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
