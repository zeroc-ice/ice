// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MA.ICPrx;
import test.Ice.inheritance.Test.MA.IC;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB.IB2Prx;

public final class ICI implements IC
{
    public ICI()
    {
    }

    @Override
    public IAPrx iaop(IAPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public ICPrx icop(ICPrx p, com.zeroc.Ice.Current current)
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
