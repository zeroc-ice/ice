// Copyright (c) ZeroC, Inc.

package test.IceDiscovery.simple;

import com.zeroc.Ice.Current;

import test.IceDiscovery.simple.Test.*;

public final class TestIntfI implements TestIntf {
    @Override
    public String getAdapterId(Current current) {
        return current.adapter
                .getCommunicator()
                .getProperties()
                .getProperty(current.adapter.getName() + ".AdapterId");
    }
}
