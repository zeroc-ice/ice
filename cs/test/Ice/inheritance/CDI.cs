// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
