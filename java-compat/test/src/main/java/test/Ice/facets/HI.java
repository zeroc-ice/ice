// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

    @Override
    public String
    callG(Ice.Current current)
    {
        return "G";
    }

    @Override
    public String
    callH(Ice.Current current)
    {
        return "H";
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
