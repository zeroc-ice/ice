// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public sealed class ICI : MA.IC_Disp
{
    public ICI()
    {
    }
    
    public override MA.IAPrx iaop(MA.IAPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override MA.ICPrx icop(MA.ICPrx p, Ice.Current current)
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
