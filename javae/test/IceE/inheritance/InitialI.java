// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class InitialI extends Test._InitialDisp
{
    public
    InitialI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _ca = Test.MA.CAPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new CAI()));
        _cb = Test.MB.CBPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new CBI()));
        _cc = Test.MA.CCPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new CCI()));
        _cd = Test.MA.CDPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new CDI()));
        _ia = Test.MA.IAPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new IAI()));
        _ib1 = Test.MB.IB1PrxHelper.uncheckedCast(
            _adapter.addWithUUID(new IB1I()));
        _ib2 = Test.MB.IB2PrxHelper.uncheckedCast(
            _adapter.addWithUUID(new IB2I()));
        _ic = Test.MA.ICPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new ICI()));
    }

    public Test.MA.CAPrx
    caop(Ice.Current current)
    {
        return _ca;
    }

    public Test.MB.CBPrx
    cbop(Ice.Current current)
    {
        return _cb;
    }

    public Test.MA.CCPrx
    ccop(Ice.Current current)
    {
        return _cc;
    }

    public Test.MA.CDPrx
    cdop(Ice.Current current)
    {
        return _cd;
    }

    public Test.MA.IAPrx
    iaop(Ice.Current current)
    {
        return _ia;
    }

    public Test.MB.IB1Prx
    ib1op(Ice.Current current)
    {
        return _ib1;
    }

    public Test.MB.IB2Prx
    ib2op(Ice.Current current)
    {
        return _ib2;
    }

    public Test.MA.ICPrx
    icop(Ice.Current current)
    {
        return _ic;
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private Test.MA.CAPrx _ca;
    private Test.MB.CBPrx _cb;
    private Test.MA.CCPrx _cc;
    private Test.MA.CDPrx _cd;
    private Test.MA.IAPrx _ia;
    private Test.MB.IB1Prx _ib1;
    private Test.MB.IB2Prx _ib2;
    private Test.MA.ICPrx _ic;
}
