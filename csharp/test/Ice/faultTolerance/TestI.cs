// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.faultTolerance;

public sealed class TestI : TestIntfDisp_
{
    public TestI()
    {
        lock (_mutex)
        {
            _p = System.Diagnostics.Process.GetCurrentProcess();
            _pid = _p.Id;
        }
    }

    private void commitSuicide()
    {
        _p.Kill();
        Thread.Sleep(5000); // Give other threads time to die.
    }

    public override void abort(Ice.Current current) => commitSuicide();

    public override void idempotentAbort(Ice.Current current) => commitSuicide();

    public override int pid(Ice.Current current)
    {
        lock (_mutex)
        {
            return _pid;
        }
    }

    public override void shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();

    private readonly object _mutex = new();
    private readonly System.Diagnostics.Process _p;
    private readonly int _pid;
}
