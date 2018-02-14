// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class InitialI : Test.InitialDisp_
{
    public InitialI(Ice.ObjectAdapter adapter)
    {
        _ca = Test.MA.CAPrxHelper.uncheckedCast(adapter.addWithUUID(new CAI()));
        _cb = Test.MB.CBPrxHelper.uncheckedCast(adapter.addWithUUID(new CBI()));
        _cc = Test.MA.CCPrxHelper.uncheckedCast(adapter.addWithUUID(new CCI()));
        _cd = Test.MA.CDPrxHelper.uncheckedCast(adapter.addWithUUID(new CDI()));
        _ia = Test.MA.IAPrxHelper.uncheckedCast(adapter.addWithUUID(new IAI()));
        _ib1 = Test.MB.IB1PrxHelper.uncheckedCast(adapter.addWithUUID(new IB1I()));
        _ib2 = Test.MB.IB2PrxHelper.uncheckedCast(adapter.addWithUUID(new IB2I()));
        _ic = Test.MA.ICPrxHelper.uncheckedCast(adapter.addWithUUID(new ICI()));
    }

    public override Test.MA.CAPrx caop(Ice.Current current)
    {
        return _ca;
    }

    public override Test.MB.CBPrx cbop(Ice.Current current)
    {
        return _cb;
    }

    public override Test.MA.CCPrx ccop(Ice.Current current)
    {
        return _cc;
    }

    public override Test.MA.CDPrx cdop(Ice.Current current)
    {
        return _cd;
    }

    public override Test.MA.IAPrx iaop(Ice.Current current)
    {
        return _ia;
    }

    public override Test.MB.IB1Prx ib1op(Ice.Current current)
    {
        return _ib1;
    }

    public override Test.MB.IB2Prx ib2op(Ice.Current current)
    {
        return _ib2;
    }

    public override Test.MA.ICPrx icop(Ice.Current current)
    {
        return _ic;
    }

    public override void  shutdown(Ice.Current current)
    {
       current.adapter.getCommunicator().shutdown();
    }

    private Test.MA.CAPrx _ca;
    private Test.MB.CBPrx _cb;
    private Test.MA.CCPrx _cc;
    private Test.MA.CDPrx _cd;
    private Test.MA.IAPrx _ia;
    private Test.MB.IB1Prx _ib1;
    private Test.MB.IB2Prx _ib2;
    private Test.MA.ICPrx _ic;
}
