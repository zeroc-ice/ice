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

public final class DI extends _DDisp
{
    public
    DI()
    {
    }

    public String
    callA(Ice.Current current)
    {
        return "A";
    }

    public String
    callB(Ice.Current current)
    {
        return "B";
    }

    public String
    callC(Ice.Current current)
    {
        return "C";
    }

    public String
    callD(Ice.Current current)
    {
        return "D";
    }
}
