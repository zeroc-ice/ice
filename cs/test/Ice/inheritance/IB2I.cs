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

public sealed class IB2I : MB.IB2_Disp
{
    public IB2I()
    {
    }
    
    public override MA.IAPrx iaop(MA.IAPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override MB.IB2Prx ib2op(MB.IB2Prx p, Ice.Current current)
    {
        return p;
    }
}
