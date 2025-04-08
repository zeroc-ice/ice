// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Current;

import test.Ice.facets.Test.C;

public final class CI implements C {
    public CI() {
    }

    @Override
    public String callA(Current current) {
        return "A";
    }

    @Override
    public String callC(Current current) {
        return "C";
    }
}
