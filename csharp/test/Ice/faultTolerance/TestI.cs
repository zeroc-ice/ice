// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using IceInternal;
using Test;

public sealed class TestI : TestIntfDisp_
{
    public TestI()
    {
        lock(this)
        {
            _p = Process.GetCurrentProcess();
            _pid = _p.Id;
        }
    }
    
    private void commitSuicide()
    {
#if !UNITY
        _p.Kill();
#endif
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
        lock(this)
        {
            return _pid;
        }
    }
    
    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private Process _p;
    private int _pid;
}
