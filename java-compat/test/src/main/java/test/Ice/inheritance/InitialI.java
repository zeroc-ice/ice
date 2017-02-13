// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.inheritance;

import test.Ice.inheritance.Test._InitialDisp;
import test.Ice.inheritance.Test.MA.CAPrx;
import test.Ice.inheritance.Test.MA.CAPrxHelper;
import test.Ice.inheritance.Test.MA.CCPrx;
import test.Ice.inheritance.Test.MA.CCPrxHelper;
import test.Ice.inheritance.Test.MA.CDPrx;
import test.Ice.inheritance.Test.MA.CDPrxHelper;
import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MA.IAPrxHelper;
import test.Ice.inheritance.Test.MA.ICPrx;
import test.Ice.inheritance.Test.MA.ICPrxHelper;
import test.Ice.inheritance.Test.MB.CBPrx;
import test.Ice.inheritance.Test.MB.CBPrxHelper;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB.IB1PrxHelper;
import test.Ice.inheritance.Test.MB.IB2Prx;
import test.Ice.inheritance.Test.MB.IB2PrxHelper;

public final class InitialI extends _InitialDisp
{
    public
    InitialI(Ice.ObjectAdapter adapter)
    {
        _ca = CAPrxHelper.uncheckedCast(adapter.addWithUUID(new CAI()));
        _cb = CBPrxHelper.uncheckedCast(adapter.addWithUUID(new CBI()));
        _cc = CCPrxHelper.uncheckedCast(adapter.addWithUUID(new CCI()));
        _cd = CDPrxHelper.uncheckedCast(adapter.addWithUUID(new CDI()));
        _ia = IAPrxHelper.uncheckedCast(adapter.addWithUUID(new IAI()));
        _ib1 = IB1PrxHelper.uncheckedCast(adapter.addWithUUID(new IB1I()));
        _ib2 = IB2PrxHelper.uncheckedCast(adapter.addWithUUID(new IB2I()));
        _ic = ICPrxHelper.uncheckedCast(adapter.addWithUUID(new ICI()));
    }

    @Override
    public CAPrx
    caop(Ice.Current current)
    {
        return _ca;
    }

    @Override
    public CBPrx
    cbop(Ice.Current current)
    {
        return _cb;
    }

    @Override
    public CCPrx
    ccop(Ice.Current current)
    {
        return _cc;
    }

    @Override
    public CDPrx
    cdop(Ice.Current current)
    {
        return _cd;
    }

    @Override
    public IAPrx
    iaop(Ice.Current current)
    {
        return _ia;
    }

    @Override
    public IB1Prx
    ib1op(Ice.Current current)
    {
        return _ib1;
    }

    @Override
    public IB2Prx
    ib2op(Ice.Current current)
    {
        return _ib2;
    }

    @Override
    public ICPrx
    icop(Ice.Current current)
    {
        return _ic;
    }

    @Override
    public void
    shutdown(Ice.Current current)
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
