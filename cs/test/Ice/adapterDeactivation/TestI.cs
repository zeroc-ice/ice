// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public sealed class TestI : Test_Disp
{
    public override void deactivate(Ice.Current current)
    {
        current.adapter.deactivate();
	System.Threading.Thread.Sleep(new System.TimeSpan(10 * 1000));
    }
}
