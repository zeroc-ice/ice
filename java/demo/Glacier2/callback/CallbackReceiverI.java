// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public final class CallbackReceiverI extends _CallbackReceiverDisp
{
    public void
    callback(Ice.Current current)
    {
        System.out.println("received callback");
    }
}
