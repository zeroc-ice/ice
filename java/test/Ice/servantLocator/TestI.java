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
    requestFailedException(Ice.Current current)
    {
    }

    public void 
    unknownUserException(Ice.Current current)
    {
    }

    public void 
    unknownLocalException(Ice.Current current)
    {
    }

    public void 
    unknownException(Ice.Current current)
    {
    }

    public void 
    localException(Ice.Current current)
    {
    }

//     public void 
//     userException(Ice.Current current)
//     {
//     }

    public void 
    javaException(Ice.Current current)
    {
    }
    
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.deactivate();
    }
}
