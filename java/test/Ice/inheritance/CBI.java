// **********************************************************************
//
// Copyright (c) 2002
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

public final class CBI extends MB.CB
{
    public
    CBI()
    {
    }

    public MA.CAPrx
    caop(MA.CAPrx p, Ice.Current current)
    {
        return p;
    }

    public MB.CBPrx
    cbop(MB.CBPrx p, Ice.Current current)
    {
        return p;
    }
}
