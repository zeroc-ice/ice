//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MB.IB2Prx;
import test.Ice.inheritance.Test.MB.IB2;

public final class IB2I implements IB2
{
    public IB2I()
    {
    }

    @Override
    public IAPrx iaop(IAPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public IB2Prx ib2op(IB2Prx p, com.zeroc.Ice.Current current)
    {
        return p;
    }
}
