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

public sealed class CDI : MA.CD
{
    public CDI()
    {
    }
    
    public override MA.CAPrx caop(MA.CAPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override MA.CCPrx ccop(MA.CCPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override MA.CDPrx cdop(MA.CDPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override MA.IAPrx iaop(MA.IAPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override MB.CBPrx cbop(MB.CBPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override MB.IB1Prx ib1op(MB.IB1Prx p, Ice.Current current)
    {
        return p;
    }
    
    public override MB.IB2Prx ib2op(MB.IB2Prx p, Ice.Current current)
    {
        return p;
    }
}
