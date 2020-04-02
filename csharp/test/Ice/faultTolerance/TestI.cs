//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using Test;

public sealed class TestIntf : ITestIntf
{
    public void abort(Ice.Current current) => Process.GetCurrentProcess().Kill();

    public void idempotentAbort(Ice.Current current) => Process.GetCurrentProcess().Kill();

    public int pid(Ice.Current current) => Process.GetCurrentProcess().Id;

    public void shutdown(Ice.Current current) => current.Adapter.Communicator.Shutdown();
}
