// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class CI extends C
{
    public
    CI()
    {
        _postUnmarshalInvoked = false;
    }

    public boolean
    postUnmarshalInvoked(Ice.Current current)
    {
        return _postUnmarshalInvoked;
    }

    public void
    ice_preMarshal()
    {
        preMarshalInvoked = true;
    }

    public void
    ice_postUnmarshal()
    {
        _postUnmarshalInvoked = true;
    }

    private boolean _postUnmarshalInvoked;
}
