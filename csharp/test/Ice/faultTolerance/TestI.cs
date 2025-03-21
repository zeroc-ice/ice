// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using Test;

public sealed class TestI : TestIntfDisp_
{
    public TestI()
    {
        lock (this)
        {
            _p = Process.GetCurrentProcess();
            _pid = _p.Id;
        }
    }

    private void commitSuicide()
    {
        _p.Kill();
        Thread.Sleep(5000); // Give other threads time to die.
    }

    public override void abort(Ice.Current current)
    {
        commitSuicide();
    }

    public override void idempotentAbort(Ice.Current current)
    {
        commitSuicide();
    }

    public override int pid(Ice.Current current)
    {
        lock (this)
        {
            return _pid;
        }
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private readonly Process _p;
    private readonly int _pid;
}
