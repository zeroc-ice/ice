// **********************************************************************
//
// Copyright (c) 2002
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

public final class TestI extends _TestDisp
{
    public void
    deactivate(Ice.Current current)
    {
        current.adapter.deactivate();
        try
        {
            Thread.sleep(1000);
        }
        catch(InterruptedException ex)
        {
        }
    }
}
