// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class CDI : Test.MA.CDDisp_
{
    public CDI()
    {
    }

    public override Test.MA.CAPrx caop(Test.MA.CAPrx p, Ice.Current current)
    {
        return p;
    }

    public override Test.MA.CCPrx ccop(Test.MA.CCPrx p, Ice.Current current)
    {
        return p;
    }

    public override Test.MA.CDPrx cdop(Test.MA.CDPrx p, Ice.Current current)
    {
        return p;
    }

    public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
    {
        return p;
    }

    public override Test.MB.CBPrx cbop(Test.MB.CBPrx p, Ice.Current current)
    {
        return p;
    }

    public override Test.MB.IB1Prx ib1op(Test.MB.IB1Prx p, Ice.Current current)
    {
        return p;
    }

    public override Test.MB.IB2Prx ib2op(Test.MB.IB2Prx p, Ice.Current current)
    {
        return p;
    }
}
