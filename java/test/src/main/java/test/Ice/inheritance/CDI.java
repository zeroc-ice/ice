// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.CAPrx;
import test.Ice.inheritance.Test.MA.CCPrx;
import test.Ice.inheritance.Test.MA._CDDisp;
import test.Ice.inheritance.Test.MA.CDPrx;
import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MB.CBPrx;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB.IB2Prx;

public final class CDI implements _CDDisp
{
    public CDI()
    {
    }

    @Override
    public CAPrx caop(CAPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public CCPrx ccop(CCPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public CDPrx cdop(CDPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public IAPrx iaop(IAPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public CBPrx cbop(CBPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public IB1Prx ib1op(IB1Prx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public IB2Prx ib2op(IB2Prx p, com.zeroc.Ice.Current current)
    {
        return p;
    }
}
