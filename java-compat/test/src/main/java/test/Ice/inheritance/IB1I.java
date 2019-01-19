//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB._IB1Disp;

public final class IB1I extends _IB1Disp
{
    public
    IB1I()
    {
    }

    @Override
    public IAPrx
    iaop(IAPrx p, Ice.Current current)
    {
        return p;
    }

    @Override
    public IB1Prx
    ib1op(IB1Prx p, Ice.Current current)
    {
        return p;
    }
}
