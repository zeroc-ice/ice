// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
