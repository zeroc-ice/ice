// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test.G;

public final class GI implements G
{
    public GI(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public String callG(com.zeroc.Ice.Current current)
    {
        return "G";
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        _communicator.shutdown();
    }

    private com.zeroc.Ice.Communicator _communicator;
}
