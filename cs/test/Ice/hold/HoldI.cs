// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;
using Test;

public sealed class HoldI : HoldDisp_
{
    public override void
    putOnHold(int seconds, Ice.Current current)
    {
        if(seconds <= 0)
        {
            current.adapter.hold();
            current.adapter.activate();
        }
        else
        {
            Debug.Assert(false); // TODO
        }
    }

    public override void
    shutdown(Ice.Current current)
    {
        current.adapter.hold();
        current.adapter.getCommunicator().shutdown();
    }
}
