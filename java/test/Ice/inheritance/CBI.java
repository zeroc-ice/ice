// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
