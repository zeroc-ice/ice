//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MA.ICPrx;
import test.Ice.inheritance.Test.MA._ICDisp;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB.IB2Prx;

public final class ICI extends _ICDisp
{
    public
    ICI()
    {
    }

    @Override
    public IAPrx
    iaop(IAPrx p, Ice.Current current)
    {
        return p;
    }

    @Override
    public ICPrx
    icop(ICPrx p, Ice.Current current)
    {
        return p;
    }

    @Override
    public IB1Prx
    ib1op(IB1Prx p, Ice.Current current)
    {
        return p;
    }

    @Override
    public IB2Prx
    ib2op(IB2Prx p, Ice.Current current)
    {
        return p;
    }
}
