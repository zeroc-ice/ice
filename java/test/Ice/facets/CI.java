// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

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
