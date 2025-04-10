// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import com.zeroc.Ice.Current;

import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MB.IB2;
import test.Ice.inheritance.Test.MB.IB2Prx;

public final class IB2I implements IB2 {
    public IB2I() {}

    @Override
    public IAPrx iaop(IAPrx p, Current current) {
        return p;
    }

    @Override
    public IB2Prx ib2op(IB2Prx p, Current current) {
        return p;
    }
}
