// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test._GDisp;

public final class GI extends _GDisp
{
    public
    GI(Ice.Communicator communicator)
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
    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
