// **********************************************************************
//
// Copyright (c) 2003
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
