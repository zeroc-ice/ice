// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class TestI : _TestIntfDisp
{
    public TestI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
	_pid = 0;
    }
    
    public override void abort(Ice.Current current)
    {
        System.Diagnostics.Process.GetCurrentProcess().Kill();
	System.Threading.Thread.Sleep(2000); // Sleep needed for Mono with RedHat 8 (no NPTL).
    }
    
    public override void idempotentAbort(Ice.Current current)
    {
        System.Diagnostics.Process.GetCurrentProcess().Kill();
	System.Threading.Thread.Sleep(2000); // Sleep needed for Mono with RedHat 8 (no NPTL).
    }
    
    public override void nonmutatingAbort(Ice.Current current)
    {
        System.Diagnostics.Process.GetCurrentProcess().Kill();
	System.Threading.Thread.Sleep(2000); // Sleep needed for Mono with RedHat 8 (no NPTL).
    }
    
    public override int pid(Ice.Current current)
    {
	lock(this)
	{
	    if(_pid == 0)
	    {
		_pid = System.Diagnostics.Process.GetCurrentProcess().Id; // Very slow call, so we cache it
	    }
	}
	return _pid;
    }
    
    public override void shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }
    
    private Ice.ObjectAdapter _adapter;
    private int _pid;
}
