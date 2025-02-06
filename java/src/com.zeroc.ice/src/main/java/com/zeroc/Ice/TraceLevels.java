// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class TraceLevels {
    TraceLevels(Properties properties) {
        dispatchCat = "Dispatch";
        networkCat = "Network";
        protocolCat = "Protocol";
        retryCat = "Retry";
        locationCat = "Locator";
        slicingCat = "Slicing";
        threadPoolCat = "ThreadPool";

        final String keyBase = "Ice.Trace.";

        dispatch = properties.getPropertyAsInt(keyBase + dispatchCat);
        network = properties.getPropertyAsInt(keyBase + networkCat);
        protocol = properties.getPropertyAsInt(keyBase + protocolCat);
        retry = properties.getPropertyAsInt(keyBase + retryCat);
        location = properties.getPropertyAsInt(keyBase + locationCat);
        slicing = properties.getPropertyAsInt(keyBase + slicingCat);
        threadPool = properties.getPropertyAsInt(keyBase + threadPoolCat);
    }

    public final int dispatch;
    public final String dispatchCat;
    public final int network;
    public final String networkCat;
    public final int protocol;
    public final String protocolCat;
    public final int retry;
    public final String retryCat;
    public final int location;
    public final String locationCat;
    public final int slicing;
    public final String threadPoolCat;
    public final int threadPool;
    public final String slicingCat;
}
