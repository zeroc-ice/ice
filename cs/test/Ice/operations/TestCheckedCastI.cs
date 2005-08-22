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
    public override Ice.Context
    getContext(Ice.Current current)
    {
        return _ctx;
    }

    public override bool
    ice_isA(string s, Ice.Current current)
    {
        _ctx = current.ctx;
	return base.ice_isA(s, current);
    }

    private Ice.Context _ctx;
}
