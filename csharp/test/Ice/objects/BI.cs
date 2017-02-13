// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class BI : B
{
    public BI()
    {
        _postUnmarshalInvoked = false;
    }

    public override bool postUnmarshalInvoked(Ice.Current current)
    {
        return _postUnmarshalInvoked;
    }

    public override void ice_preMarshal()
    {
        preMarshalInvoked = true;
    }

    public override void ice_postUnmarshal()
    {
        _postUnmarshalInvoked = true;
    }

    private bool _postUnmarshalInvoked;
}
