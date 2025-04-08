// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Current;

import test.Ice.facets.Test.D;

public final class DI implements D {
    public DI() {
    }

    @Override
    public String callA(Current current) {
        return "A";
    }

    @Override
    public String callB(Current current) {
        return "B";
    }

    @Override
    public String callC(Current current) {
        return "C";
    }

    @Override
    public String callD(Current current) {
        return "D";
    }
}
