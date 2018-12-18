// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.inheritance;

import test.Ice.inheritance.Test.Initial;
import test.Ice.inheritance.Test.MA.CAPrx;
import test.Ice.inheritance.Test.MA.CCPrx;
import test.Ice.inheritance.Test.MA.CDPrx;
import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MA.ICPrx;
import test.Ice.inheritance.Test.MB.CBPrx;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB.IB2Prx;

public final class InitialI implements Initial
{
    public InitialI(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _ca = CAPrx.uncheckedCast(adapter.addWithUUID(new CAI()));
        _cb = CBPrx.uncheckedCast(adapter.addWithUUID(new CBI()));
        _cc = CCPrx.uncheckedCast(adapter.addWithUUID(new CCI()));
        _cd = CDPrx.uncheckedCast(adapter.addWithUUID(new CDI()));
        _ia = IAPrx.uncheckedCast(adapter.addWithUUID(new IAI()));
        _ib1 = IB1Prx.uncheckedCast(adapter.addWithUUID(new IB1I()));
        _ib2 = IB2Prx.uncheckedCast(adapter.addWithUUID(new IB2I()));
        _ic = ICPrx.uncheckedCast(adapter.addWithUUID(new ICI()));
    }

    @Override
    public CAPrx caop(com.zeroc.Ice.Current current)
    {
        return _ca;
    }

    @Override
    public CBPrx cbop(com.zeroc.Ice.Current current)
    {
        return _cb;
    }

    @Override
    public CCPrx ccop(com.zeroc.Ice.Current current)
    {
        return _cc;
    }

    @Override
    public CDPrx cdop(com.zeroc.Ice.Current current)
    {
        return _cd;
    }

    @Override
    public IAPrx iaop(com.zeroc.Ice.Current current)
    {
        return _ia;
    }

    @Override
    public IB1Prx ib1op(com.zeroc.Ice.Current current)
    {
        return _ib1;
    }

    @Override
    public IB2Prx ib2op(com.zeroc.Ice.Current current)
    {
        return _ib2;
    }

    @Override
    public ICPrx icop(com.zeroc.Ice.Current current)
    {
        return _ic;
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private CAPrx _ca;
    private CBPrx _cb;
    private CCPrx _cc;
    private CDPrx _cd;
    private IAPrx _ia;
    private IB1Prx _ib1;
    private IB2Prx _ib2;
    private ICPrx _ic;
}
