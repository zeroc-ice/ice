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

public final class ICI extends MA._ICDisp
{
    public
    ICI()
    {
    }

    public MA.IAPrx
    iaop(MA.IAPrx p, Ice.Current current)
    {
        return p;
    }

    public MA.ICPrx
    icop(MA.ICPrx p, Ice.Current current)
    {
        return p;
    }

    public MB.IB1Prx
    ib1op(MB.IB1Prx p, Ice.Current current)
    {
        return p;
    }

    public MB.IB2Prx
    ib2op(MB.IB2Prx p, Ice.Current current)
    {
        return p;
    }
}
