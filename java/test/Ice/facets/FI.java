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

public final class FI extends _FDisp
{
    public
    FI()
    {
    }

    public String
    callE(Ice.Current current)
    {
        return "E";
    }

    public String
    callF(Ice.Current current)
    {
        return "F";
    }
}
