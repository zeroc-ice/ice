// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public final class RetryI extends _RetryDisp
{
    public
    RetryI()
    {
    }

    public void
    op(boolean kill, Ice.Current current)
    {
        if(kill)
        {
            current.con.close(true);
        }
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
