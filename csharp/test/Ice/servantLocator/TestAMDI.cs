// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

    public override void 
    unknownExceptionWithServantException_async(AMD_TestIntf_unknownExceptionWithServantException cb, 
                                               Ice.Current current)
    {
        cb.ice_exception(new Ice.ObjectNotExistException());
    }

    public override void impossibleException_async(AMD_TestIntf_impossibleException cb, bool @throw,
                                                   Ice.Current current)
    {
        if(@throw)
        {
            cb.ice_exception(new Test.TestImpossibleException());
        }
        else
        {
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly iuf finished throws.
            //
            cb.ice_response("Hello");
        }
    }

    public override void intfUserException_async(AMD_TestIntf_intfUserException cb, bool @throw, Ice.Current current)
    {
        if(@throw)
        {
            cb.ice_exception(new Test.TestIntfUserException());
        }
        else
        {
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly iuf finished throws.
            //
            cb.ice_response("Hello");
        }
    }

    public override void asyncResponse_async(AMD_TestIntf_asyncResponse cb, Ice.Current current)
    {
        cb.ice_response();
        throw new Ice.ObjectNotExistException();
    }

    public override void asyncException_async(AMD_TestIntf_asyncException cb, Ice.Current current)
    {
        cb.ice_exception(new Test.TestIntfUserException());
        throw new Ice.ObjectNotExistException();
    }

    public override void shutdown_async(AMD_TestIntf_shutdown cb, Ice.Current current)
    {
        current.adapter.deactivate();
        cb.ice_response();
    }
}
