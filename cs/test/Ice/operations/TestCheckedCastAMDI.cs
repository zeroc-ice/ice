// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class TestCheckedCastI : Test._TestCheckedCastDisp
{
    public override void
    getContext_async(Test.AMD_TestCheckedCast_getContext cb, Ice.Current current)
    {
        cb.ice_response(_ctx);
    }

    public void
    setContext(Ice.Context ctx)
    {
        _ctx = ctx;
    }

    private Ice.Context _ctx;
}
