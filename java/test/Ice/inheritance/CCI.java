// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public final class CCI extends MA.CC
{
    public
    CCI()
    {
    }

    public MA.CAPrx
    caop(MA.CAPrx p, Ice.Current current)
    {
        return p;
    }

    public MA.CCPrx
    ccop(MA.CCPrx p, Ice.Current current)
    {
        return p;
    }

    public MB.CBPrx
    cbop(MB.CBPrx p, Ice.Current current)
    {
        return p;
    }
}
