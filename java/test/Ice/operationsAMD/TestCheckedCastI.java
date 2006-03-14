// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class TestCheckedCastI extends Test._TestCheckedCastDisp
{
    public void
    getContext_async(Test.AMD_TestCheckedCast_getContext cb, Ice.Current current)
    {
        cb.ice_response(_ctx);
    }

    public boolean
    ice_isA(String s, Ice.Current current)
    {
        _ctx = current.ctx;
        return super.ice_isA(s, current);
    }

    private java.util.Map _ctx;
}
