// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class TestCheckedCastI : Test._TestCheckedCastDisp
{
    public override Ice.Context
    getContext(Ice.Current current)
    {
        return _ctx;
    }

    public void
    setContext(Ice.Context ctx)
    {
        _ctx = ctx;
    }

    private Ice.Context _ctx;
}
