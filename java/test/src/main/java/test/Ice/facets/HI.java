//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

import test.Ice.facets.Test.H;

public final class HI implements H
{
    public HI(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public String callG(com.zeroc.Ice.Current current)
    {
        return "G";
    }

    @Override
    public String callH(com.zeroc.Ice.Current current)
    {
        return "H";
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        _communicator.shutdown();
    }

    private com.zeroc.Ice.Communicator _communicator;
}
