// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class TestCheckedCastI extends Test._TestCheckedCastDisp
{
    public java.util.Map
    getContext(Ice.Current current)
    {
        return _ctx;
    }

    public void
    setContext(java.util.Map ctx)
    {
        _ctx = ctx;
    }

    private java.util.Map _ctx;
}
