// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
