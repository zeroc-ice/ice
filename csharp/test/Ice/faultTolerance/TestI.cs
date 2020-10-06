// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Diagnostics;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public sealed class TestIntf : ITestIntf
    {
        public void Abort(Current current) => Process.GetCurrentProcess().Kill();

        public int Pid(Current current) => System.Environment.ProcessId;

        public void Shutdown(Current current) => _ = current.Adapter.Communicator.ShutdownAsync();
    }
}
