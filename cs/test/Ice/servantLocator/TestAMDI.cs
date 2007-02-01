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
    public override void requestFailedException_async(AMD_TestIntf_requestFailedException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void unknownUserException_async(AMD_TestIntf_unknownUserException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void unknownLocalException_async(AMD_TestIntf_unknownLocalException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void unknownException_async(AMD_TestIntf_unknownException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void localException_async(AMD_TestIntf_localException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void userException_async(AMD_TestIntf_userException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void csException_async(AMD_TestIntf_csException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public override void shutdown_async(AMD_TestIntf_shutdown cb, Ice.Current current)
    {
        current.adapter.deactivate();
        cb.ice_response();
    }
}
