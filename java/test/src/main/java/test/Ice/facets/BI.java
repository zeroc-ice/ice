// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Current;

import test.Ice.facets.Test.B;

public final class BI implements B {
    public BI() {}

    @Override
    public String callA(Current current) {
        return "A";
    }

    @Override
    public String callB(Current current) {
        return "B";
    }
}
