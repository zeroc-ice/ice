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

public final class InitialI extends _InitialDisp
{
    public
    InitialI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _ca = MA.CAPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new CAI()));
        _cb = MB.CBPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new CBI()));
        _cc = MA.CCPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new CCI()));
        _cd = MA.CDPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new CDI()));
        _ia = MA.IAPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new IAI()));
        _ib1 = MB.IB1PrxHelper.uncheckedCast(
            _adapter.addWithUUID(new IB1I()));
        _ib2 = MB.IB2PrxHelper.uncheckedCast(
            _adapter.addWithUUID(new IB2I()));
        _ic = MA.ICPrxHelper.uncheckedCast(
            _adapter.addWithUUID(new ICI()));
    }

    public MA.CAPrx
    caop(Ice.Current current)
    {
        return _ca;
    }

    public MB.CBPrx
    cbop(Ice.Current current)
    {
        return _cb;
    }

    public MA.CCPrx
    ccop(Ice.Current current)
    {
        return _cc;
    }

    public MA.CDPrx
    cdop(Ice.Current current)
    {
        return _cd;
    }

    public MA.IAPrx
    iaop(Ice.Current current)
    {
        return _ia;
    }

    public MB.IB1Prx
    ib1op(Ice.Current current)
    {
        return _ib1;
    }

    public MB.IB2Prx
    ib2op(Ice.Current current)
    {
        return _ib2;
    }

    public MA.ICPrx
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
    private MA.CAPrx _ca;
    private MB.CBPrx _cb;
    private MA.CCPrx _cc;
    private MA.CDPrx _cd;
    private MA.IAPrx _ia;
    private MB.IB1Prx _ib1;
    private MB.IB2Prx _ib2;
    private MA.ICPrx _ic;
}
