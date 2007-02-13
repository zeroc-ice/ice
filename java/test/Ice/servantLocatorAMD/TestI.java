// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public final class TestI extends _TestIntfDisp
{
    public void 
    requestFailedException_async(AMD_TestIntf_requestFailedException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public void 
    unknownUserException_async(AMD_TestIntf_unknownUserException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public void 
    unknownLocalException_async(AMD_TestIntf_unknownLocalException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public void 
    unknownException_async(AMD_TestIntf_unknownException cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public void 
    localException_async(AMD_TestIntf_localException cb, Ice.Current current)
    {
        cb.ice_response();
    }

//     public void 
//     userException_async(AMD_TestIntf_userException cb, Ice.Current current)
//     {
//        cb.ice_response();
//     }

    public void 
    javaException_async(AMD_TestIntf_javaException cb, Ice.Current current)
    {
        cb.ice_response();
    }
    
    public void
    shutdown_async(AMD_TestIntf_shutdown cb, Ice.Current current)
    {
        current.adapter.deactivate();
        cb.ice_response();
    }
}
