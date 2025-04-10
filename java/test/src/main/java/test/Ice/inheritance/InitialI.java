// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.inheritance.Test.Initial;
import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MA.ICPrx;
import test.Ice.inheritance.Test.MB.IB1Prx;
import test.Ice.inheritance.Test.MB.IB2Prx;

public final class InitialI implements Initial {
    public InitialI(ObjectAdapter adapter) {
        _ia = IAPrx.uncheckedCast(adapter.addWithUUID(new IAI()));
        _ib1 = IB1Prx.uncheckedCast(adapter.addWithUUID(new IB1I()));
        _ib2 = IB2Prx.uncheckedCast(adapter.addWithUUID(new IB2I()));
        _ic = ICPrx.uncheckedCast(adapter.addWithUUID(new ICI()));
    }

    @Override
    public IAPrx iaop(Current current) {
        return _ia;
    }

    @Override
    public IB1Prx ib1op(Current current) {
        return _ib1;
    }

    @Override
    public IB2Prx ib2op(Current current) {
        return _ib2;
    }

    @Override
    public ICPrx icop(Current current) {
        return _ic;
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private final IAPrx _ia;
    private final IB1Prx _ib1;
    private final IB2Prx _ib2;
    private final ICPrx _ic;
}
