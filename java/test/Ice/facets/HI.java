// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

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
