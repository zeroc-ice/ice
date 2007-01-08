// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public final class GI extends _GDisp
{
    public
    GI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public String
    callG(Ice.Current current)
    {
        return "G";
    }

    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
