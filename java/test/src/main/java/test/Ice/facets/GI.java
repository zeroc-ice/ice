// Copyright (c) ZeroC, Inc.

package test.Ice.facets;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;

import test.Ice.facets.Test.G;

public final class GI implements G {
    public GI(Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public String callG(Current current) {
        return "G";
    }

    @Override
    public void shutdown(Current current) {
        _communicator.shutdown();
    }

    private final Communicator _communicator;
}
