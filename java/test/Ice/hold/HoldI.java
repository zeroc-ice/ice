// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public final class HoldI extends _HoldDisp
{
    public void
    putOnHold(int seconds, Ice.Current current)
    {
        if(seconds <= 0)
        {
            current.adapter.hold();
            current.adapter.activate();
        }
        else
        {
            assert(false); // TODO
        }
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.hold();
        current.adapter.getCommunicator().shutdown();
    }
}
