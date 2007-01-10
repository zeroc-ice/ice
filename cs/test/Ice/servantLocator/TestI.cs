// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class TestI : TestIntfDisp_
{
    public override void requestFailedException(Ice.Current current)
    {
    }

    public override void unknownUserException(Ice.Current current)
    {
    }

    public override void unknownLocalException(Ice.Current current)
    {
    }

    public override void unknownException(Ice.Current current)
    {
    }

    public override void localException(Ice.Current current)
    {
    }

    public override void userException(Ice.Current current)
    {
    }

    public override void csException(Ice.Current current)
    {
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.deactivate();
    }
}
