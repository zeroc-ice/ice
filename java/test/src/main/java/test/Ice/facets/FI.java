// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Current;

import test.Ice.facets.Test.F;

public final class FI implements F {
    public FI() {
    }

    @Override
    public String callE(Current current) {
        return "E";
    }

    @Override
    public String callF(Current current) {
        return "F";
    }
}
