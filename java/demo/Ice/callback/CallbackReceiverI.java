// **********************************************************************
//
// Copyright (c) 2001
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

public final class CallbackReceiverI extends CallbackReceiver
{
    public void
    callback(Ice.Current current)
    {
        System.out.println("received callback");
    }
}
