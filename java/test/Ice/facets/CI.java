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

public final class CI extends _CDisp
{
    public
    CI()
    {
    }

    public String
    callA(Ice.Current current)
    {
        return "A";
    }

    public String
    callC(Ice.Current current)
    {
        return "C";
    }
}
