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


public sealed class IB1I : MB.IB1_Disp
{
    public IB1I()
    {
    }
    
    public override MA.IAPrx iaop(MA.IAPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override MB.IB1Prx ib1op(MB.IB1Prx p, Ice.Current current)
    {
        return p;
    }
}
