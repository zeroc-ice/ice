// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Ice version 0.0.1

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
