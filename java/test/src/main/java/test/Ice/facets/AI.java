// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Current;

import test.Ice.facets.Test.A;

public final class AI implements A {
    public AI() {}

    @Override
    public String callA(Current current) {
        return "A";
    }
}
