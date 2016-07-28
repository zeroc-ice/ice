// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class DI : D
{
    public override void ice_preMarshal()
    {
        preMarshalInvoked = true;
    }

    public override void ice_postUnmarshal()
    {
        postUnmarshalInvoked = true;
    }
}
