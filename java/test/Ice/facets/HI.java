// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test._HDisp;

public final class HI extends _HDisp
{
    public
    HI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public String
    callG(Ice.Current current)
    {
        return "G";
    }

    public String
    callH(Ice.Current current)
    {
        return "H";
    }

    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
