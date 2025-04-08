// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Current;

import test.Ice.facets.Test.E;

public final class EI implements E {
    public EI() {
    }

    @Override
    public String callE(Current current) {
        return "E";
    }
}
