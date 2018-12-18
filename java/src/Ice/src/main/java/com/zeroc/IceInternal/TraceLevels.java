// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

public final class TraceLevels
{
    TraceLevels(com.zeroc.Ice.Properties properties)
    {
        networkCat = "Network";
        protocolCat = "Protocol";
        retryCat = "Retry";
        locationCat = "Locator";
        slicingCat = "Slicing";
        threadPoolCat = "ThreadPool";

        final String keyBase = "Ice.Trace.";

        network = properties.getPropertyAsInt(keyBase + networkCat);
        protocol = properties.getPropertyAsInt(keyBase + protocolCat);
        retry = properties.getPropertyAsInt(keyBase + retryCat);
        location = properties.getPropertyAsInt(keyBase + locationCat);
        slicing = properties.getPropertyAsInt(keyBase + slicingCat);
        threadPool = properties.getPropertyAsInt(keyBase + threadPoolCat);
    }

    final public int network;
    final public String networkCat;
    final public int protocol;
    final public String protocolCat;
    final public int retry;
    final public String retryCat;
    final public int location;
    final public String locationCat;
    final public int slicing;
    final public String threadPoolCat;
    final public int threadPool;
    final public String slicingCat;
}
