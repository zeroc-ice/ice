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
    public override void deactivate(Ice.Current current)
    {
        current.adapter.deactivate();
	System.Threading.Thread.Sleep(new System.TimeSpan(10 * 1000));
    }
}
