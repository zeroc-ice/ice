// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.IA;
import test.Ice.inheritance.Test.MA.IAPrx;

public final class IAI implements IA {
    public IAI() {}

    @Override
    public IAPrx iaop(IAPrx p, com.zeroc.Ice.Current current) {
        return p;
    }
}
