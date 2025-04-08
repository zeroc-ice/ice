// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import com.zeroc.Ice.Current;

import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MB.IB1;
import test.Ice.inheritance.Test.MB.IB1Prx;

public final class IB1I implements IB1 {
    public IB1I() {
    }

    @Override
    public IAPrx iaop(IAPrx p, Current current) {
        return p;
    }

    @Override
    public IB1Prx ib1op(IB1Prx p, Current current) {
        return p;
    }
}
