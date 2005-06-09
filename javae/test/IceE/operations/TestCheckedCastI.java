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
    public java.util.Hashtable
    getContext(Ice.Current current)
    {
        return _ctx;
    }

    public void
    setContext(java.util.Hashtable ctx)
    {
        _ctx = ctx;
    }

    private java.util.Hashtable _ctx;
}
