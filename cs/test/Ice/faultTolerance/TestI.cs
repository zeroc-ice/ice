// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public sealed class TestI : Test_Disp
{
    public TestI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }
    
    public override void abort(Ice.Current current)
    {
        System.Environment.Exit(1);
    }
    
    public override void idempotentAbort(Ice.Current current)
    {
        System.Environment.Exit(1);
    }
    
    public override void nonmutatingAbort(Ice.Current current)
    {
        System.Environment.Exit(1);
    }
    
    public override int pid(Ice.Current current)
    {
        return (int)((System.DateTime.Now.Ticks - 621355968000000000) / 10000 % 65535);
    }
    
    public override void shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }
    
    private Ice.ObjectAdapter _adapter;
}
