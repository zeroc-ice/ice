// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import com.zeroc.Ice.Current;

import test.Ice.inheritance.Test.MA.IA;
import test.Ice.inheritance.Test.MA.IAPrx;

public final class IAI implements IA {
    public IAI() {
    }

    @Override
    public IAPrx iaop(IAPrx p, Current current) {
        return p;
    }
}
