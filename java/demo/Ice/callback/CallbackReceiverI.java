// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
