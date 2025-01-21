// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import test.Ice.facets.Test.A;

public final class AI implements A {
    public AI() {}

    @Override
    public String callA(com.zeroc.Ice.Current current) {
        return "A";
    }
}
